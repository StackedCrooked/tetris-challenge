#ifndef STM_TRANSACTION_GROUP
#define STM_TRANSACTION_GROUP

#include "backend.hpp"

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace stm {
	namespace backend {
		struct shared_base;
	}

	namespace frontend {
		template <typename manager_type>
		struct tx_group;

		// scoped helper class ensuring version counter lock is released on destruction
		template <typename manager_type>
		struct group_lock_guard {
			explicit group_lock_guard(tx_group<manager_type>& group);
			void confirm() throw();
			version_field_t version() { return commit_version; }
		private:
			group_lock_guard(const group_lock_guard&);
			group_lock_guard& operator=(const group_lock_guard&);

			tx_group<manager_type>* group;
			boost::lock_guard<boost::shared_mutex> lock;
			const version_field_t commit_version;

		};

		template <typename manager_type>
		struct tx_group {
			tx_group() : version_clock() {}
			
			version_field_t get_current_version() throw() { 
				boost::shared_lock<boost::shared_mutex> l(mtx);
				return atomic_ops::read(&version_clock);
			}

			boost::shared_mutex& version_lock() { return mtx; }

		private:
			friend struct group_lock_guard<manager_type>;

			// can only be called if we've already locked, since it's private and only called from noncopyable group_lock_guard
			version_field_t increment() { 
				version_field_t res; 
				res = ver_ops::plus_one(atomic_ops::read(&version_clock));
				atomic_ops::write(res, &version_clock);
				atomic_ops::compiler_memory_barrier();

				return res;
			}
			
			// returns the version to be used when committing the locking transaction (current version + 1) - throws if already locked
			version_field_t reserve_commit_version() {
				version_field_t cur_ver = atomic_ops::read(&version_clock);
				return ver_ops::plus_one(cur_ver);

			}
			// can only be called if we've already locked, since it's private and only called from noncopyable group_lock_guard
			void apply_commit_version() throw() {
				increment();
			}

			version_field_t version_clock;

			boost::shared_mutex mtx;
		};

		template <typename manager_type>
		inline group_lock_guard<manager_type>::group_lock_guard(tx_group<manager_type>& group) 
			: group(&group)
			, lock(group.version_lock())
			, commit_version(group.reserve_commit_version())
		{}

		template <typename manager_type>
		inline void group_lock_guard<manager_type>::confirm() throw(){
			group->apply_commit_version();
		}
	}
}
#endif
