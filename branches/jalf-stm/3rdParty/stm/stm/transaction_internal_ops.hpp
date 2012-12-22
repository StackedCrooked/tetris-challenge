#ifndef STM_TRANSACTION_INTERNAL_OPS_HPP
#define STM_TRANSACTION_INTERNAL_OPS_HPP

#include "backend.hpp"

namespace stm {
	namespace frontend {
		// attempts to lock all objects opened for writing, in preparation for committing
		template <typename lookup_iter>
		lookup_iter lock(version_field_t tx_version, lookup_iter first, lookup_iter last){
			lookup_iter it = first;
			while (it != last && (it->first)->lock_for_commit(tx_version)){
				++it;
			}
			if (it != last){
				lookup_iter nit = first;
				while (nit != it){
					(nit->first)->release_unchanged();
					++nit;
				}
				return it;
			}
			return last;
		}

		// Helper class: Calls release when it goes out of scope
		template <typename manager_type, typename iter_type>
		struct release_writer_guard : boost::noncopyable {
			release_writer_guard(manager_type& manager, iter_type first, iter_type last) : manager(manager), first(first), last(last), commit_version(), is_committing(false) {}
			~release_writer_guard(){
				// release write list
				if (is_committing) {
					manager.release_commit_lock_on_commit(first, last, commit_version);
				}
				else {
					manager.release_commit_lock_on_rollback(first, last);
				}
			}

			void committing(version_field_t commit_version) {
				this->commit_version = commit_version;
				this->is_committing = true;
			}

			manager_type& manager;
			iter_type first;
			iter_type last;
			version_field_t commit_version;
			bool is_committing;
		};

		// MSVC doesn't implement no-capture lambda to function pointer conversion, so leave this for now
		template <typename T>
		void direct_assign(const void* src, void* dest) {
			const T& s = *reinterpret_cast<const T*>(src);
			T& d = *reinterpret_cast<T*>(dest);
			d = s;
		}
	}
}

#endif
