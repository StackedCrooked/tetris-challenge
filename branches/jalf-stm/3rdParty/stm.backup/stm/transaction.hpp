#ifndef STM_TRANSACTION_HPP
#define STM_TRANSACTION_HPP

//FIXME: we shouldnt need to include this
#include "transaction_internal.hpp"
#include "transaction_internal_ops.hpp"

namespace stm {

	struct transaction;
	namespace frontend{
	template <typename backend_type, typename T>
		struct shared_internal_common;

		struct transaction_internal;

		void commit(transaction_internal& txb);
		bool in_orelse(transaction_internal& txb);
		bool is_outer(transaction_internal& txb);
		void add_snapshot(transaction_internal& tx, void (*func)(const void*,void*), const void* src, void* dest);

	}
	namespace detail {
		frontend::transaction_internal& inner_tx(transaction& tx);
	}

	struct transaction  {
		transaction(frontend::transaction_internal& inner) : inner(inner) {}

		void abort();
		//{
		//	detail::scoped_access_version guard;
		//	transaction_internal::rollback();
		//	throw abort_exception();
		//}

		void retry();
		//{
		//	detail::scoped_access_version guard;
		//	throw retry_exception();
		//}

		template <typename T>
		void snapshot(const T* src, T& dest) {
			add_snapshot(inner, frontend::direct_assign<T>, src, &dest);
		}

		size_t age() const; // do we actually want this to be part of the api?
		//{
		//	return static_cast<const frontend::transaction_internal*>(this)->get_age();
		//}

	private:
		//friend transaction& detail::outer_tx(frontend::transaction_internal& tx);
		friend frontend::transaction_internal& detail::inner_tx(transaction& tx);
		transaction(const transaction&);

		template <typename backend_type, typename T>
		friend struct frontend::shared_internal_common;

		typedef frontend::transaction_internal internal_type;
		frontend::transaction_internal& inner;

		template <typename T>
		friend class shared;
	};

	namespace detail {
		// helper functions converting between a transaction and its frontend equivalent

		//inline transaction& outer_tx(frontend::transaction_internal& tx) {
		//	return static_cast<transaction&>(tx);
		//}
		inline frontend::transaction_internal& inner_tx(transaction& tx) {
			return tx.inner;
		}
	}
}

#endif
