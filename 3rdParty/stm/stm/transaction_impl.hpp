#ifndef H_TRANSACTION_IMPL_3428B5756CD58B429022FB2B11C81CBC
#define H_TRANSACTION_IMPL_3428B5756CD58B429022FB2B11C81CBC

#include "transaction.hpp"
#include "transaction_internal.hpp"

namespace stm {
	STM_LIB_FUNC void transaction::abort() {
		detail::scoped_access_version guard;
		inner.rollback();
		throw abort_exception();
	}

	STM_LIB_FUNC void transaction::retry() {
		detail::scoped_access_version guard;
		throw retry_exception();
	}
}

#endif
