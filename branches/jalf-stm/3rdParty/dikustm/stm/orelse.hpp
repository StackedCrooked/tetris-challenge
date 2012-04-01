#ifndef STM_ORELSE_HPP
#define STM_ORELSE_HPP

#include "atomic.hpp"

namespace stm {
  // helper class defining a compound transaction consisting of the two transactions passed to the constructor, combined using the orelse operation
  template <typename first_type, typename second_type>
  struct orelse_wrapper {
    orelse_wrapper(first_type first, second_type second) : first(first), second(second) {}

    void operator()(transaction&){
      try {
        detail::atomic<void, first_type>(first, true);
      }
      catch (const retry_exception&) {
        detail::atomic<void, second_type>(second, true);
      }
    }

    first_type first;
    second_type second;
  };
  // operator || for combining an orelse with another transaction
  template <typename lhs1_tx, typename lhs2_tx, typename rhs_tx>
  orelse_wrapper<orelse_wrapper<lhs1_tx, lhs2_tx>, rhs_tx> operator || (orelse_wrapper<lhs1_tx, lhs2_tx> lhs, rhs_tx rhs) {
    return orelse_wrapper<orelse_wrapper<lhs1_tx, lhs2_tx>, rhs_tx>(lhs, rhs);
  }
  // operator || for combining an orelse with another transaction
  template <typename lhs_tx, typename rhs1_tx, typename rhs2_tx>
  orelse_wrapper<lhs_tx, orelse_wrapper<rhs1_tx, rhs2_tx> > operator || (lhs_tx lhs, orelse_wrapper<rhs1_tx, rhs2_tx> rhs) {
    return orelse_wrapper<lhs_tx, orelse_wrapper<rhs1_tx, rhs2_tx> >(lhs, rhs);
  }

  // user-facing orelse function
  template <typename lhs_tx_t, typename rhs_tx_t>
  orelse_wrapper<lhs_tx_t, rhs_tx_t> orelse(lhs_tx_t lhs, rhs_tx_t rhs) { return orelse_wrapper<lhs_tx_t, rhs_tx_t>(lhs, rhs); }
}

#endif
