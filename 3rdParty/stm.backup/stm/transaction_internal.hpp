#ifndef STM_TRANSACTION_INTERNAL_HPP
#define STM_TRANSACTION_INTERNAL_HPP

#include "transaction_group.hpp"
#include "transaction_manager.hpp"
#include "exception.hpp"
#include "transaction_internal_ops.hpp"
#include "list_buffer.hpp" 
#include "config.hpp"
#include "transaction.hpp"

#include <vector>

namespace stm {
	namespace frontend {
		struct transaction_internal {
			typedef config::default_buffer buffer_type;
			typedef transaction_manager<buffer_type> manager_type;
			typedef tx_group<manager_type> group_type;

			transaction_internal(manager_type& mgr, version_field_t tx_ver, bool is_orelse = false) throw()
				: tx_version(tx_ver)
				, startpos_rw(mgr.buffer().generic_pos())
				, startpos_r(mgr.buffer().ptr_pos())
				, snapshot_offset(mgr.snapshots.size())
				, mgr(mgr)
				, is_orelse(is_orelse)
				, is_dead()
			{
				detail::scoped_access_version guard;
				manager().lut_delims.push_back(manager().buffer_lookup.size());
			}

			~transaction_internal() throw() { 
				detail::scoped_access_version guard;
				if (!is_dead){
					rollback();
				}

				manager().lut_delims.pop_back();

				assert(is_dead);
			}

			// if locking fails during commit, this is called. Currently just throws an exception, possibly after a short delay. 
			// A more complex implementation could block until transaction's readset is modified
			void recover_from_failed_lock(){
				//if (age > config::retries_before_blocking) {
				//	yield();
				//}
				throw conflict_on_commit(conflict_on_commit::lock_failed);
			}

			void full_commit() { 
				auto& man = manager();
				auto clear_lookup = scope_guard([&man]() { man.buffer_lookup.clear(); });
				auto clear_buffer = scope_guard([=, &man](){
					// release readers
					man.release_read_lock(startpos_r, tx_version);
					// destroy writers
					man.destroy(man.buffer_lookup.begin() + man.lut_delims.back());
					// pop readers and writers
					auto new_r = startpos_r;
					auto new_rw = startpos_rw;
					man.buffer().release_ptrs(new_r);
					man.buffer().release_generic(new_rw);
				});

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
				release_writer_guard<manager_type, decltype(manager().buffer_lookup.begin())> release_writers(manager(), writes_first, writes_last); 

				// Create scoped object which releases lock on version counter on destruction
				group_lock_guard<manager_type> lock(mgr.group()); 

				// validate objects
				version_field_t commit_version = lock.version();
				if (!manager().validate(startpos_r, tx_version)){ // still need to validate readers, I guess. Is that what causes hang?
					throw conflict_on_commit(conflict_on_commit::validate_failed);
				}
				// guaranteed completion from here on
				// now, tell releaser commit version, and let it switch from release_unchanged to release_updated.
				release_writers.committing(commit_version);

				{ detail::scoped_access_version guard; }

				// before applying changes (because apply might use destructive moves), we need to write out snapshot
				std::for_each(manager().snapshots.begin() + snapshot_offset, manager().snapshots.end(), [](const snapshot_val& val){ val.assign(val.src, val.dest); });
				manager().snapshots.resize(snapshot_offset);
				// now apply speculative changes
				std::for_each(writes_first, writes_last, [commit_version](std::pair<backend::shared_base*, void*> entry){ 
					backend::buffer_traits<buffer_type>::assign(static_cast<backend::buffer_traits<buffer_type>::entry_type*>(entry.second));
					atomic_ops::compiler_memory_barrier();
					backend::update_and_flip(*entry.first, commit_version);
				});

				// and signal to the version counter that our commit was successful and version counter should be incremented
				lock.confirm();
			}

			void nested_commit() throw() {
				auto& man = manager();

				auto first = man.buffer_lookup.begin() + man.lut_delims.back();
				auto last = man.buffer_lookup.end();

				// goddamn complicated and unintuitive
				// but if we're losing the whole tail dealy, all we need is: first remaining item (becomes new top)
				// need to be aware that we may have nulls in index buffer (because entries past the first may have been deleted, and we can't compact
				// so we're iterating overe lut, which contains id (shared_base*) -> buffer_node (iterator_rw)
				// that means we're not necessarily iterating in original order? So why? It's not even necessary in nested tx.
				// and *if* we iterate out of order, a simple accumulate is no good for determining new_end.
				// tænkeboks?
				// ok, write new version assuming we use buffer's own iters

				// since we're no longer traversing in insertion order, we just need to know if *any* object survived, not what the *first* surviving object was
				auto has_survivor = std::accumulate(first, last, false, [&man](bool has_survivor, std::pair<backend::shared_base*, void*>& cur) -> bool {
					// if the object was opened in a parent transaction, update it, and delete current copy
					// todo: these funcs should take object pointer and convert to whatever structure is used internally. Oops! Can't be done for indirection-based.
					auto* entry = static_cast<backend::buffer_traits<buffer_type>::entry_type*>(cur.second);
					if (entry->outer != NULL){
						backend::buffer_traits<buffer_type>::assign(entry);
						backend::buffer_traits<buffer_type>::destroy(entry);
						cur.first = NULL;
						return has_survivor;
					}
					// otherwise, we have an object which must survive the commit
					return true;
				});

				if (!has_survivor) {
					man.buffer().release_generic(startpos_rw);
				}
				// do compaction here if we choose to reimplement it

				// this is a second pass. Might be nice if we could roll it into the above accum
				// remove from lut if object has a reopen_tail
				auto remove_from = std::remove_if(first, last, [](std::pair<backend::shared_base*, void*> val) {
					return val.first == NULL; 
				});
				man.buffer_lookup.erase(remove_from, last);

				// now sort from lut_delims.back() to end
				auto pred = [](std::pair<backend::shared_base*, void*> lhs, std::pair<backend::shared_base*, void*> rhs) { return lhs.first < rhs.first; };
				
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
				}
				else {
					full_commit();
				}
				is_dead = true;
			}

			void rollback() throw() {
                auto& mgr = manager();
				mgr.buffer_lookup.resize(mgr.lut_delims.back());
				mgr.release_read_lock(startpos_r, tx_version);
				mgr.destroy(mgr.buffer_lookup.begin() + mgr.lut_delims.back());
				mgr.buffer().release_ptrs(startpos_r);
				mgr.buffer().release_generic(startpos_rw);
				is_dead = true;
			}

			template <typename T>
			typename backend::buffer_traits<buffer_type>::entry_type* open_rw(backend::shared_base& handle, const T& obj, destroy_type destroy, assign_type assign) {
				return manager().open_rw(handle, obj, destroy, assign);
			}
			template <typename T>
			typename backend::buffer_traits<buffer_type>::entry_type* open_rw_from_parent(backend::shared_base& handle, void* outer_open, destroy_type destroy, assign_type assign) {
				return manager().template open_rw_from_parent<T>(handle, outer_open, destroy, assign);
			}

			void open_r(const backend::shared_base& handle) const throw() {
				manager().open_r(handle);
			}

			version_field_t version() throw() { return tx_version; }

			manager_type& manager() throw() {
				return mgr;
			}
			manager_type& manager() const throw() {
				return mgr;
			}

			// find in current tx. If not found, return the iterator where object should be inserted.
			manager_type::lut_container::iterator find_nearest_in_current(backend::shared_base* obj) const{
				return std::lower_bound(manager().buffer_lookup.begin() +  manager().lut_delims.back(), manager().buffer_lookup.end(), obj, [](std::pair<const backend::shared_base* const, const void* const> pair, const backend::shared_base* obj) { return std::less<const backend::shared_base* const>()(pair.first, obj); });
			}
			
			void* find_in_parent(backend::shared_base* shared) {
				return manager().find(manager().lut_delims.begin(), manager().lut_delims.end()-1, shared);
			}
			void* find(const backend::shared_base* shared) const {
				return manager().find(manager().lut_delims.begin(), manager().lut_delims.end(), shared);
			}

			bool is_outer() {
				return manager().depth() == 1;
			}

			bool is_in_orelse() { return manager().is_orelse(); }

			void add_snapshot(void (*func)(const void*,void*), const void* src, void* dest){
				manager().snapshots.push_back(snapshot_val(func, src, dest));
			}

		private:
			transaction_internal(const transaction_internal&);
			transaction_internal& operator=(const transaction_internal&);

			const version_field_t tx_version;
			const buffer_type::generic_position startpos_rw;
			const buffer_type::ptr_position startpos_r;
			size_t snapshot_offset; // this transaction's snapshot registrations start at manager().snapshots[snapshot_offset]
			manager_type& mgr;
			bool is_orelse;
			bool is_dead;
		};
		inline void add_snapshot(transaction_internal& tx, void (*func)(const void*,void*), const void* src, void* dest){
			tx.add_snapshot(func, src, dest);
		}

		inline void commit(transaction_internal& txb) {
			txb.commit();
		}

		inline bool in_orelse(transaction_internal& txb) {
			return txb.is_in_orelse();
		}

		inline bool is_outer(transaction_internal& txb) {
			return txb.is_outer();
		}
	}

	struct transaction;
	namespace detail {
		//inline transaction& outer_tx(frontend::transaction_internal& tx);
		inline frontend::transaction_internal& inner_tx(transaction& tx);
	}
}

#endif
