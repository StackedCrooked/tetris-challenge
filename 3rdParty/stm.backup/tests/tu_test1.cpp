#include <stm.hpp>
#include <catch.hpp>

void* test_helper();

// Just to make sure inlining and other trickery hasn't broken anything
TEST_CASE("TUs share context", "Two separate TUs should return the same context address")
{
	void* tu1_addr = &stm::frontend::default_tx_group();
	void* tu2_addr = test_helper();
	REQUIRE(tu1_addr == tu2_addr);
}

