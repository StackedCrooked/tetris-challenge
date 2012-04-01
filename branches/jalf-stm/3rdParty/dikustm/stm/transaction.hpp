#ifndef STM_TRANSACTION_HPP
#define STM_TRANSACTION_HPP

#include "transaction_internal.hpp"
#include "shared_internal_common.hpp"

namespace stm {
  struct transaction : private frontend::transaction_internal {
    transaction(uint32_t retry_count = 0, bool is_orelse = false) : transaction_internal(retry_count, is_orelse) {}

    void abort() {
      transaction_internal::rollback();
      throw abort_exception();
    }

    void retry() {
      throw retry_exception();
    }

    template <typename T>
    void snapshot(const T* src, T& dest){
      transaction_internal::snapshot(src, dest);
    }

  private:
    friend transaction& detail::outer_tx(frontend::transaction_internal& tx);
    friend frontend::transaction_internal& detail::inner_tx(transaction& tx);
    template <typename backend_type, typename T>
    friend struct frontend::shared_internal_common;

    typedef frontend::transaction_internal internal_type;

    template <typename T>
    friend class shared;
  };

  namespace detail {
    // helper functions converting between a transaction and its frontend equivalent
    inline transaction& outer_tx(frontend::transaction_internal& tx) {
      return static_cast<transaction&>(tx);
    }
    inline frontend::transaction_internal& inner_tx(transaction& tx) {
      return *ptr_cast<frontend::transaction_internal*>(&tx);
    }
  }
}

#endif
