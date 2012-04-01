#ifndef STM_TRANSACTION_GROUP
#define STM_TRANSACTION_GROUP

#include "transaction_manager.hpp"
#include "fixed_array_buffer.hpp"

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

namespace stm {
	namespace backend {
		struct shared_base;
	}

	namespace frontend {
    template <typename manager_t>
		struct tx_group;

    typedef transaction_manager<backend::fixed_array_buffer<> > default_manager_type;

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

			// get the thread's transaction manager associated with this transaction group
			manager_t& manager() throw() {
				if (thread_manager.get() == NULL) {
					thread_manager.reset(new manager_t());
				}
				return *thread_manager;
			}

      version_field_t get_current_version() throw() { 
        return static_cast<derived*>(this)->do_get_current_version();
      }


    protected:
			version_field_t do_get_current_version() throw() { 
				return version_clock;
			}
			// can only be called if we've already locked, since it's private and only called from noncopyable group_lock_guard
			version_field_t increment() { return atomic_ops::increment(version_clock); }

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

			boost::thread_specific_ptr<manager_t> thread_manager;
			version_field_t version_clock;
		};

    template <typename manager_t>
		struct tx_group : tx_group_base<tx_group<manager_t>, manager_t> {
			version_field_t do_get_current_version() throw() { 
				boost::shared_lock<boost::shared_mutex> l(mtx);
				return tx_group_base<tx_group<manager_t>, manager_t>::do_get_current_version();
			}

			version_field_t do_reserve_commit_version() {
				return tx_group_base<tx_group<manager_t>, manager_t>::do_get_current_version() + 1;
			}
			void do_apply_commit_version() throw() {
				increment();
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

		// the default transaction group to use if no other is specified
		tx_group<default_manager_type>& default_tx_group() throw();
	}
}
#endif
