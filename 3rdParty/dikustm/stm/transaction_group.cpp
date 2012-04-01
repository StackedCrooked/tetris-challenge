#include "transaction_group.hpp"

namespace stm {
	namespace frontend {
		namespace {
			tx_group<default_manager_type> default_group;
		}
		tx_group<default_manager_type>& default_tx_group() throw() {
			return default_group;
		}
	}
}
