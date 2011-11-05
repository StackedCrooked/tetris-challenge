#ifndef STM_TRANSACTION_MANAGER_OPS_HPP
#define STM_TRANSACTION_MANAGER_OPS_HPP

#include "backend.hpp"

namespace stm {
	namespace frontend {
		// helper functor - can be replaced with c++0x lambda when supported
    // designed for use with std::accumulate. takes the current result and the object to be validated.
    // returns true if previous result is true and validation passes
		struct validator {
			validator(version_field_t tx_version) throw() : tx_version(tx_version) {}
			bool operator()(bool res, backend::shared_base* base) throw() {
				return res && ver_ops::shared_valid_in_tx(base->version(), tx_version);
			}

			version_field_t tx_version;
		};
	}
}

#endif
