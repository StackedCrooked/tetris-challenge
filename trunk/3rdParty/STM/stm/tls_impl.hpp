#ifndef H_TLS_IMPL_450B85CEF3E61540B9AC6A867D22EF33
#define H_TLS_IMPL_450B85CEF3E61540B9AC6A867D22EF33

#include "transaction_manager.hpp"
#include <deque>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

namespace stm {
	namespace frontend {
		
		STM_LIB_FUNC default_manager_type*& get_mgrptr() {
			static STM_TLS default_manager_type* tls_txmgr;
			return tls_txmgr;
		}

		struct tls_data {
			std::deque<default_manager_type> mgrs;
			boost::mutex mtx;

			default_manager_type* insert() {
				boost::lock_guard<boost::mutex> lock(mtx);
				mgrs.push_back(default_manager_type());
				return &mgrs.back();
			}
		};

		STM_LIB_FUNC tls_data& get_tls() {
			static tls_data data;
			return data;
		}

		STM_LIB_FUNC default_manager_type& get_manager() throw() {
			default_manager_type*& ptr = get_mgrptr();
			if (ptr == NULL) {
				ptr = get_tls().insert();
			}
			return *ptr;
		}
	}
}

#endif
