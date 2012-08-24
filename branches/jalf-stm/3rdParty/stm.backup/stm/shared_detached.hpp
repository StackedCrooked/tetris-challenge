#ifndef STM_SHARED_DETACHED_HPP
#define STM_SHARED_DETACHED_HPP

#include "shared_internal_detached.hpp"

namespace stm {
	template <typename buffer_type>
	struct transaction_manager;

	template <typename T>
	class shared_detached;

	namespace detail {
		template <typename T>
		frontend::shared_internal_detached<T>& open_shared(shared_detached<T>& shd);
	}

  // Implements transactional semantics on an object stored elsewhere. The metadata is "detached" from the object it protects.
  // Intended for use to transaction-enable contiguous sequences of objects, or existing objects that should not be copied to new locations.
	template <typename T>
	class shared_detached : private frontend::shared_internal_detached<T> {
		typedef typename frontend::shared_internal_detached<T>::non_const_val non_const_val;

		template <typename U>
		friend frontend::shared_internal_detached<U>& detail::open_shared(shared_detached<U>& shd);

	public:
		shared_detached() : frontend::shared_internal_detached<T>(){}
		explicit shared_detached(non_const_val& val) : frontend::shared_internal_detached<T>(val){}

		void set_source(non_const_val& val) { frontend::shared_internal_detached<T>::set_source(val); }

		non_const_val& open_rw(transaction& tx) { 
			return static_cast<frontend::shared_internal_detached<T>*>(this)->open_rw(tx);
//			return this->frontend::shared_internal_detached<T>::open_rw(tx);
		} 
		const T& open_r(transaction& tx) { 
			return this->frontend::shared_internal_detached<T>::open_r(tx);
		}
	};

	namespace detail {
    // Internal helper function used to retrieve the frontend class shared_detached forwards to
		template <typename T>
		frontend::shared_internal_detached<T>& open_shared(shared_detached<T>& shd) {
			return static_cast<frontend::shared_internal_detached<T>& >(shd);
		}
	}
}

#endif
