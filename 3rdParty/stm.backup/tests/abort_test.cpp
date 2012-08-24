#include <stm.hpp>
#include <catch.hpp>

TEST_CASE("abort/onthrow", "Aborts when exception is thrown in transaction")
{
	REQUIRE_THROWS(stm::atomic([](stm::transaction&)
	{
		throw std::exception();
	}));	
}

