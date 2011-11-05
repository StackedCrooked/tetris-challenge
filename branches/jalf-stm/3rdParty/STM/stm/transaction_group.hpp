#ifndef STM_TRANSACTION_GROUP
#define STM_TRANSACTION_GROUP

#include "transaction_manager.hpp"
#include "backend.hpp"

#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "tls.hpp"

namespace stm {
	namespace backend {
		struct shared_base;
	}

	namespace frontend {
		template <typename manager_t>
		struct tx_group;

		// scoped helper class ensuring version counter lock is released on destruction
		template <typename manager_t = default_manager_type>
		struct group_lock_guard : private boost::noncopyable {
			explicit group_lock_guard(tx_group<manager_t>& group);
			void confirm() throw();
			version_field_t version() { return commit_version; }
		private:
			tx_group<manager_t>* group;			
			boost::lock_guard<boost::shared_mutex> lock;
			const version_field_t commit_version;

		};

		// CRTP base class, for implementing multiple version counter/locking schemes
		template <typename derived, typename manager_t>
		struct tx_group_base {
			tx_group_base() : version_clock() {}

			version_field_t get_current_version() throw() { 
				return static_cast<derived*>(this)->do_get_current_version();
			}


		protected:
			version_field_t do_get_current_version() throw() { 
				return atomic_ops::read(&version_clock);

				//return atomic_ops::cas(version_clock, 0, 0);

			}
			// can only be called if we've already locked, since it's private and only called from noncopyable group_lock_guard
			version_field_t increment() { 
				version_field_t res; 
				res = ver_ops::plus_one(atomic_ops::read(&version_clock));
				atomic_ops::write(res, &version_clock);
				atomic_ops::memory_barrier();

				return res;
			}

		private:
			friend struct group_lock_guard<manager_t>;
			// returns the version to be used when committing the locking transaction (current version + 1) - throws if already locked
			version_field_t reserve_commit_version() {
				return static_cast<derived*>(this)->do_reserve_commit_version();
			}
			// can only be called if we've already locked, since it's private and only called from noncopyable group_lock_guard
			void apply_commit_version() throw() {
				static_cast<derived*>(this)->do_apply_commit_version();
			}

			version_field_t version_clock;
		};

		template <typename manager_t>
		struct tx_group : tx_group_base<tx_group<manager_t>, manager_t> {
			version_field_t do_get_current_version() throw() { 
				boost::shared_lock<boost::shared_mutex> l(mtx);
				return tx_group_base<tx_group<manager_t>, manager_t>::do_get_current_version();
			}

			version_field_t do_reserve_commit_version() {
				version_field_t cur_ver = tx_group_base<tx_group<manager_t>, manager_t>::do_get_current_version();
				return ver_ops::plus_one(cur_ver);
			}
			void do_apply_commit_version() throw() {
				tx_group_base<tx_group<manager_t>, manager_t>::increment();
			}

			boost::shared_mutex& version_lock() { return mtx; }

		private:
			boost::shared_mutex mtx;
		};

		template <typename manager_t>
		inline group_lock_guard<manager_t>::group_lock_guard(tx_group<manager_t>& group) 
			: group(&group)
			, lock(group.version_lock())
			, commit_version(group.reserve_commit_version())
		{}

		template <typename manager_t>
		inline void group_lock_guard<manager_t>::confirm() throw(){
			group->apply_commit_version();
		}

		inline tx_group<default_manager_type>& default_tx_group() throw() {
			static tx_group<default_manager_type> default_group;
			return default_group;
		}

	}
}
#endif
