#include <stm.hpp>

void* test_helper() {
	return &stm::frontend::default_tx_group();
}

