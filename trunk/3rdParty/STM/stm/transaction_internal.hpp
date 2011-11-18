#ifndef STM_TRANSACTION_INTERNAL_HPP
#define STM_TRANSACTION_INTERNAL_HPP

#include "transaction_group.hpp"
#include "transaction_manager.hpp"
#include "exception.hpp"
#include "transaction_internal_ops.hpp"
#include "config.hpp"

#include <boost/noncopyable.hpp>
#include <vector>

namespace stm {
	namespace frontend {
		struct transaction_internal : private boost::noncopyable {
			typedef backend::fixed_array_buffer tx_local_t;
			typedef transaction_manager<tx_local_t> manager_t;
			typedef tx_group<manager_t> group_type;
			typedef manager_t::tx_marker marker_t;

			transaction_internal(uint32_t retry_count = 0, bool is_orelse = false, group_type& grp = default_tx_group()) throw()
				: grp(grp)
				, mgr(get_manager())
				, is_live(true)
				, tx_start(manager().begin())
				, tx_version(manager().is_new_tx_outer() ? this->group().get_current_version() : manager().last_version)
				, age(retry_count)
				, is_orelse(is_orelse)
				, snapshot_offset(manager().snapshots.size())
			{
				detail::scoped_access_version guard;
				manager().lut_delims.push_back(manager().buffer_lookup.size());
				if (is_outer()) { // todo: branch should be unnecessary here
					manager().last_version = tx_version;
				}
				if (is_orelse){
					manager().orelse_starting();
				}
			}

			~transaction_internal() throw() { 
				detail::scoped_access_version guard;

				--manager().depth(); // depth seems unnecessary now that we have lut tables
				if (is_orelse){
					manager().orelse_ending();
				}

				if (is_live){
					rollback();
				}

				manager().lut_delims.pop_back();

				assert(!is_live);
			}

			// if locking fails during commit, this is called. Currently just throws an exception, possibly after a short delay. 
			// A more complex implementation could block until transaction's readset is modified
			void recover_from_failed_lock(){
				// todo: what do we want with this?
				if (age > config::retries_before_blocking) {
					yield();
				}
				throw conflict_on_commit(conflict_on_commit::lock_failed);
			}
			// Main commit function, for non-nested transactions
			void full_commit() { 
				auto& man = manager();
				auto clear_lookup = on_destroy([&man]() { man.buffer_lookup.clear(); });
				pop_guard<manager_t> pop(manager(), tx_end(), tx_version, is_live);

				auto writes_first = manager().buffer_lookup.begin() + manager().lut_delims.back();
				auto writes_last = manager().buffer_lookup.end();
				// keep trying to lock objects, until we either succeed, or give up and throw an exception
				for (;;){
					auto res = lock(tx_version, writes_first, writes_last);
					if (res == manager().buffer_lookup.end()){
						break;
					}
					
					recover_from_failed_lock();
				}

				// Create scoped object which releases write list on destruction
				release_writer_guard<manager_t, decltype(manager().buffer_lookup.begin())> release_writers(manager(), writes_first, writes_last); 

				// Create scoped object which releases lock on version counter on destruction
				group_lock_guard<> lock(group()); 

				// validate objects
				version_field_t commit_version = lock.version();
				if (!manager().validate(tx_end(), tx_version)){ // still need to validate readers, I guess. Is that what causes hang?
					throw conflict_on_commit(conflict_on_commit::validate_failed);
				}
				// guaranteed completion from here on
				// now, tell releaser commit version, and let it switch from release_unchanged to release_updated.
				release_writers.committing(commit_version);

				{ 
					detail::scoped_access_version guard;
				}
				// before applying changes (because apply might use destructive moves), we need to write out snapshot
				std::for_each(manager().snapshots.begin() + snapshot_offset, manager().snapshots.end(), apply_snapshot());
				manager().snapshots.resize(snapshot_offset);
				// now apply speculative changes
				std::for_each(writes_first, writes_last, apply(commit_version));

				// and signal to the version counter that our commit was successful and version counter should be incremented
				lock.confirm();
			}

			void nested_commit() throw() {
				auto& man = manager();

				auto first = man.buffer_lookup.begin() + man.lut_delims.back();
				auto last = man.buffer_lookup.end();
				backend::metadata* first_remaining = NULL;

				// todo: can we ditch the whole linked list farce? What are we trying to do anyway? Just save a few bytes?
				// remove from underlying buffer. Currently depends on tail member for linked list traversal. todo: could/should traverse lut instead?
				std::accumulate(first, last, static_cast<backend::metadata*>(NULL), [&first_remaining](backend::metadata* acc, std::pair<backend::shared_base*, backend::metadata*> cur) -> backend::metadata* {
					// if the object was opened in a parent transaction, update it, and delete current copy
					if (cur.second->reopen_tail != NULL) {
						// if a previous record exists, adjust its tail pointer to skip current element
						if (acc != NULL) {
							acc->tail = cur.second->tail;
						}
						cur.second->assign(cur.second, cur.second->reopen_tail);
						cur.second->destroy(cur.second);
						return acc;
					}
					// if the object is opened for the first time, leave it as is
					// and, if no earlier object has been found, record it as the new first
					else {
						first_remaining = first_remaining == NULL ? cur.second : NULL;
						return cur.second;
					}
				});	
				if (first_remaining == NULL) {
					auto te = tx_end().second;
					first_remaining = reinterpret_cast<backend::metadata*>(&*te);
				}
				man.pop(manager_t::iterator_rw(first_remaining));

				// remove from lut if object has a reopen_tail
				auto remove_from = std::remove_if(first, last, [](std::pair<backend::shared_base*, backend::metadata*> val) { return val.second->reopen_tail != NULL; });
				man.buffer_lookup.erase(remove_from, last);

				// now sort from lut_delims.back() to end
				auto pred = [](std::pair<backend::shared_base*, backend::metadata*> lhs, std::pair<backend::shared_base*, backend::metadata*> rhs) { return lhs.first < rhs.first; };
				//std::sort(man.buffer_lookup.begin() + *(man.lut_delims.rbegin()+1), man.buffer_lookup.end(), pred);
				
				// (probably) optimized sorting. Rather than O(n lgn) sort, use the fact that we are merging two sorted ranges.
				// can easily check if sorting is unnecessary, and if it is, we can expand vector a bit, and do a merge-sort to the new elements and then move back in O(n) 
				auto l = man.buffer_lookup.begin() + *(man.lut_delims.rbegin()+1);
				auto m = man.buffer_lookup.begin() + man.lut_delims.back();
				auto r = man.buffer_lookup.end();
				// if either range is empty, or it is already sorted (last element of lhs is less than first element of rhs)
				if (m == l || r == m || pred(*(m-1), *m)) {
					return;
				}
				ptrdiff_t span = r - l;

				man.buffer_lookup.resize(man.buffer_lookup.size() + span);
				l = man.buffer_lookup.begin() + *(man.lut_delims.rbegin()+1);
				m = man.buffer_lookup.begin() + man.lut_delims.back();
				r = l + span;

				std::merge(l, m, m, r, r, pred);
				std::move(r, man.buffer_lookup.end(), l);
				man.buffer_lookup.erase(r, man.buffer_lookup.end());

				// might be worth doing a bit of benchmarking on this.
			}

			void commit(){
				if (!is_outer()) {
					nested_commit();
					is_live = false;
					return;
				}

				full_commit();
			}

			void rollback() throw() {
				manager().buffer_lookup.resize(manager().lut_delims.back());
				manager().release_read_lock(tx_end().first, tx_version);
				manager().destroy(tx_end().second);
				manager().pop(tx_end());
				is_live = false;
			}

			template <typename T>
			backend::metadata* open_rw(backend::shared_base& handle, const T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
				return manager().open_rw(handle, obj, destroy, assign);
			}
			template <typename T>
			backend::metadata* open_rw_from_parent(backend::shared_base& handle, backend::metadata* outer_open, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
				return manager().template open_rw_from_parent<T>(handle, outer_open, destroy, assign);
			}

			void open_r(backend::shared_base& handle) throw() {
				manager().open_r(handle);
			}

			version_field_t version() throw() { return tx_version; }

			size_t get_age() const {
				return age;
			}

			manager_t& manager() throw() {
				return mgr;
			}

			// todo: these should probably be scrapped or replaced with luts?
			marker_t tx_end() throw() { return tx_start; }
			marker_t tx_begin() throw() { return manager().begin(); }

			// find in current tx. If not found, return the iterator where object should be inserted.
			manager_t::lut_container::iterator find_nearest_in_current(backend::shared_base* obj){
				return std::lower_bound(manager().buffer_lookup.begin() +  manager().lut_delims.back(), manager().buffer_lookup.end(), obj, [](std::pair<const backend::shared_base* const, const backend::metadata* const> pair, const backend::shared_base* obj) { return std::less<const backend::shared_base* const>()(pair.first, obj); });
			}
			
			backend::metadata* find_in_parent(backend::shared_base* shared) {
				return manager().find(manager().lut_delims.begin(), manager().lut_delims.end()-1, shared);
			}
			backend::metadata* find(backend::shared_base* shared) {
				return manager().find(manager().lut_delims.begin(), manager().lut_delims.end(), shared);
			}

			group_type& group() { return grp; }

			bool is_outer() {
				return manager().depth() == 1;
			}

			bool is_in_orelse() { return manager().is_orelse(); }

			void add_snapshot(void (*func)(const void*,void*), const void* src, void* dest){
				manager().snapshots.push_back(snapshot_val(func, src, dest));
			}

			template <typename T>
			void snapshot(const T* src, T& dest){
				add_snapshot(direct_assign<T>, src, &dest);
			}

		private:
			group_type& grp;
			manager_t& mgr;

			bool is_live;
			const manager_t::tx_marker tx_start;
			const version_field_t tx_version;
			uint32_t age;
			bool is_orelse;
			size_t snapshot_offset; // this transaction's snapshot registrations start at manager().snapshots[snapshot_offset]


			struct apply_snapshot {
				void operator()(const snapshot_val& val){
					val.f(val.src, val.dest);
				}
			};
		};
	}

	struct transaction;
	namespace detail {
		inline transaction& outer_tx(frontend::transaction_internal& tx);
		inline frontend::transaction_internal& inner_tx(transaction& tx);
	}
}

#endif
