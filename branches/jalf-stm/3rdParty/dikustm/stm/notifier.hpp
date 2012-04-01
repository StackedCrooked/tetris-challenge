#ifndef STM_NOTIFIER_HPP
#define STM_NOTIFIER_HPP

#include "transaction_group.hpp"
#include "shared_base.hpp"

namespace stm {
	namespace frontend {
    // intended to enable intelligent blocking, allowing transactions to block if they're unable to lock an object,
    // and be woken up once the object is released
    template <typename manager_t>
    struct notifier {
			explicit notifier(tx_group<manager_t>& ) {}
      // a shared object has been unlocked
			void operator()(backend::shared_base&){}

			~notifier() {	}
		};
	}
}
#endif
