#ifndef STM_SHARED_HPP
#define STM_SHARED_HPP

#include "shared_internal.hpp"
#include "atomic.hpp"

namespace stm {
  struct transaction;

  template <typename buffer_t>
  struct transaction_manager;

  template <typename T>
  class shared;

  namespace detail {
    template <typename T>
    frontend::shared_internal<T>& open_shared(shared<T>& shd);
  }

  // Main shared class -- Transaction-enables an object stored in the class, preventing non-transactional accesses to it
  template <typename T>
  class shared : private frontend::shared_internal<T> {
    typedef typename frontend::shared_internal<T>::non_const_val non_const_val;

    template <typename U>
    friend frontend::shared_internal<U>& detail::open_shared(shared<U>& shd);

  public:
    explicit shared(const T& val) : frontend::shared_internal<T>(val){}

	template <typename internal_type, typename U>
	explicit shared(const frontend::shared_internal_common<internal_type, U>& shd) : frontend::shared_internal<T>(shd){}

	template <typename U>
	shared& operator=(const shared<U>& other) {
		shared& self = *this;
		stm::atomic([&](stm::transaction& tx) {
			tx.open_rw(self) = tx.open_r(other);
		});
		return *this;
	}

    non_const_val& open_rw(transaction& tx) { 
      return frontend::shared_internal<T>::open_rw(tx);
    } 
    const T& open_r(transaction& tx) { 
      return frontend::shared_internal<T>::open_r(tx);
    }
  };

  namespace detail {
    // Internal helper function used to retrieve the frontend class shared_detached forwards to
    template <typename T>
    frontend::shared_internal<T>& open_shared(shared<T>& shd) {
      return static_cast<frontend::shared_internal<T>& >(shd);
    }
  }
}

#endif
