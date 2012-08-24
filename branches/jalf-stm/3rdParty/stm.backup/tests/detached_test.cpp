#include <stm.hpp>

#include <catch.hpp>

namespace stm {
	template <typename iter_type>
	struct shared_range : private backend::shared_base {
		shared_range(iter_type first, iter_type last) : first(first), last(last) {}

		iter_type first;
		iter_type last;
	};
}

struct simple_tx {
	simple_tx(stm::shared_detached<int>& d) : d(d) {};
	void operator()(stm::transaction& tx){
		int& i = d.open_rw(tx);
		++i;
	}

	stm::shared_detached<int>& d;
};

TEST_CASE("detached", "simple tests/syntax demos for detached shared objects")
{
	int i = 0;
	int r[] = {0, 1, 2, 3, 4};

	{
		stm::shared_detached<int> si(i); // stores T&
		// modify i. Since this is the only transaction running, it should commit successfully,
		// meaning changes should be stored in secondary buffer first time around.
		stm::atomic(simple_tx(si)); 
		REQUIRE(i == 0);
	} // let si go out of scope. That should force primary to be updated
	
	REQUIRE(i == 1);

	{
		stm::shared_detached<int> si(i); // stores T&

		// modify i. Since this is the only transaction running, it should commit successfully
		// first update should make secondary buffer the primary
		stm::atomic(simple_tx(si)); 
		// second update should make primary active, and then no flushing should be necessary to get updated value
		stm::atomic(simple_tx(si)); 
		REQUIRE(i == 3);
	} // let si go out of scope. Check that we didn't flush and overwrite primary with outdated secondary value
	REQUIRE(i == 3);



	stm::shared_range<int*> sr(r, r+5); // stores copies of iters
}
