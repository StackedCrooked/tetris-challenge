#ifndef STM_TRANSACTION_INTERNAL_HPP
#define STM_TRANSACTION_INTERNAL_HPP

#include "writelist.hpp"
#include "notifier.hpp"
#include "transaction_group.hpp"
#include "transaction_manager.hpp"
#include "exception.hpp"
#include "transaction_internal_ops.hpp"
#include "config.hpp"

#include <boost/noncopyable.hpp>
#include <deque>

namespace stm {
	namespace frontend {
		template <typename transaction_type>
		struct is_reopen {
			is_reopen(transaction_type& tx) : tx(tx){ }
      // given an object obj, has previously been opened by the same transaction (not a parent)
			bool operator()(backend::shared_base** obj){
				backend::metadata* meta = ptr_cast<backend::metadata*>(obj);
				return meta->reopen_tail != NULL && tx.is_reopen_in_tx(meta);
			}

			transaction_type& tx;
		};

		struct transaction_internal : private boost::noncopyable {
			typedef backend::fixed_array_buffer<> tx_local_t;
			typedef transaction_manager<tx_local_t> manager_t;
			typedef tx_group<manager_t> group_type;
			typedef manager_t::tx_marker marker_t;
      typedef notifier<manager_t> notifier_type;

			transaction_internal(uint32_t retry_count = 0, bool is_orelse = false, group_type& grp = default_tx_group()) throw()
				: grp(grp)
				, mgr(grp.manager())
				, is_live(true)
				, tx_start(manager().begin())
				, tx_version(manager().is_new_tx_outer() ? this->group().get_current_version() : manager().last_version)
				, age(retry_count)
				, is_orelse(is_orelse)
			{
				if (is_outer()) {
					manager().last_version = tx_version;
				}
				if (is_orelse){
					manager().orelse_starting();
				}
			}

			~transaction_internal() throw() { 
				--manager().depth();
				if (is_orelse){
					manager().orelse_ending();
				}

				if (is_live){
					rollback();
				}
				assert(!is_live);
			}

      // if locking fails during commit, this is called. Currently just throws an exception, possibly after a short delay. 
      // A more complex implementation could block until transaction's readset is modified
			void recover_from_failed_lock(backend::shared_base** ){
				if (age > config::retries_before_blocking) {
					yield();
				}
				throw conflict_on_commit(conflict_on_commit::lock_failed);
			}
      // Main commit function, for non-nested transactions
			template <writelist_policies write_policy>
			void full_commit() { 
				typedef writelist<transaction_internal, write_policy> writelist_t;
				typedef typename writelist_t::iterator write_iter;
        // generate a sorted list of objects opened for writing
        writelist_t writes(*this);

				notifier<manager_t> ntf(group());
				pop_guard<manager_t, notifier<manager_t> > pop(manager(), tx_end(), tx_version, is_live, ntf);
        // keep trying to lock objects, until we either succeeds, or give up and throw an exception
				for (;;){
					write_iter res = lock(writes.begin(), writes.end(), ntf);
					if (res == writes.end()){
						break;
					}
					recover_from_failed_lock(*res);
				}

				// Create scoped object which releases write list on destruction
				release_writer_guard<manager_t, write_iter, notifier<manager_t> > release_writers(manager(), writes.begin(), writes.end(), ntf);
				
        // Create scoped object which releases lock on version counter on destruction
        group_lock_guard<> lock(group()); 

        // validate objects
				version_field_t commit_version = lock.version();
				if (!manager().validate(tx_end(), tx_version)){
					throw conflict_on_commit(conflict_on_commit::validate_failed);
				}
				// guaranteed completion from here on
				// before applying changes (because apply might use destructive moves), we need to write out snapshot
				std::for_each(snapshots.begin(), snapshots.end(), apply_snapshot());
				// now apply speculative changes
        std::for_each(writes.begin(), writes.end(), apply(commit_version));
        // and signal to the version counter that our commit was successful and version counter should be incremented
				lock.confirm();
			}

			void nested_commit() throw() {
				backend::metadata* prev = ptr_cast<backend::metadata*>(&*manager().begin().second);
				backend::metadata* first = NULL;
				std::for_each(tx_begin().second, tx_end().second, nested_apply(first, prev));
				if (first == NULL) {
					first = ptr_cast<backend::metadata*>(&*tx_end().second);
				}
				manager().pop(manager_t::iterator_rw(first));
			}

			void commit(){
				if (!is_outer()) {
					nested_commit();
					is_live = false;
					return;
				}

				size_t writeset_size = manager().writeset_size(tx_end().second);

				if (config::sort_write_list) {
					if (writeset_size > config::max_writeset_size_for_stack){
						full_commit<sorted_on_heap>();
					}
					else {
						full_commit<sorted_on_stack>();
					}
				}
				else {
					//inner_commit<unsorted>(); // currently has one less level of indirection than the others,
          // so until all versions are given the same signature, don't use this one
					full_commit<sorted_on_heap>();
				}
			}

			void rollback() throw() {
				notifier<manager_t> ntf(group());
				manager().release(tx_end().first, tx_version, ntf);
				manager().destroy(tx_end().second);
				manager().pop(tx_end());
				is_live = false;
			}

			template <typename T>
			T& open_rw(backend::shared_base& handle, backend::metadata* outer_open, T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
				return manager().open_rw(handle, outer_open, obj, destroy, assign);
			}

			void open_r(backend::shared_base& handle) throw() {
				manager().open_r(handle);
			}

      // Has an object been opened before by this transaction
			bool is_reopen_in_tx(backend::metadata* obj) throw() {
				manager_t::iterator_rw start(obj);
				return std::find_if(start, tx_end().second, equal_metadata(obj)) != tx_end().second;
			}

			version_field_t version() throw() { return tx_version; }


			manager_t& manager() throw() {
				return mgr;
			}

			marker_t tx_end() throw() { return tx_start; }
			marker_t tx_begin() throw() { return manager().begin(); }

			backend::metadata* find_in_tx(backend::shared_base* shared) {
				return manager().find(tx_begin().second, tx_end().second, shared);
			}
			backend::metadata* find_in_parent(backend::shared_base* shared) {
				return manager().find(tx_end().second, manager().end().second, shared);
			}
			backend::metadata* find(backend::shared_base* shared) {
				return manager().find(tx_begin().second, manager().end().second, shared);
			}

			group_type& group() { return grp; }

			bool is_outer() {
				return manager().depth() == 1;
			}

			bool is_in_orelse() { return manager().is_orelse(); }

			void add_snapshot(void (*func)(const void*,void*), const void* src, void* dest){
				snapshots.push_back(snapshot_val(func, src, dest));
			}

			template <typename T>
			void snapshot(const T* src, T& dest){
				add_snapshot(direct_assign<T>, src, &dest);
			}

		private:
			struct snapshot_val {
				typedef void (*assign_func)(const void*, void*);
				snapshot_val(assign_func f, const void* src, void* dest) : f(f), src(src), dest(dest) {}

				assign_func f;
				const void* src;
				void* dest;
			};

			group_type& grp;
			manager_t& mgr;
			std::deque<snapshot_val> snapshots;

			bool is_live;
			const manager_t::tx_marker tx_start;
			const version_field_t tx_version;
			uint32_t age;
			bool is_orelse;

			struct apply_snapshot {
				void operator()(const snapshot_val& val){
					val.f(val.src, val.dest);
				}
			};

      // helper functor for searching for objects in buffer
			struct equal_metadata {
				equal_metadata(backend::metadata* obj) : obj(obj) {}
				bool operator()(backend::shared_base*& sh){
					backend::metadata* meta = ptr_cast<backend::metadata*>(&sh);
					return meta->reopen_tail == obj;
				}
				backend::metadata* obj;
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
