#include <stm/shared_base.hpp>
#include <stm/shared.hpp>
#include <boost/test/unit_test.hpp>


namespace {

	struct dummy_functor {
		template <typename T>
		void operator()(T){}
	};
	struct shared_fixture {
		shared_fixture () : shd(42) {}

		stm::frontend::shared_internal<int> shd;
	};
}

BOOST_AUTO_TEST_SUITE( Shared )

BOOST_FIXTURE_TEST_CASE( acquire_test, shared_fixture )
{
	dummy_functor ntf;
	// if no one have acquired it yet, this should succeed
	BOOST_CHECK(shd.lock_for_commit(ntf));
	// and then subsequent attempts to acquire should fail
	BOOST_CHECK(!shd.lock_for_commit(ntf));
}

BOOST_FIXTURE_TEST_CASE( release_test, shared_fixture )
{
	dummy_functor ntf;
	shd.lock_for_commit(ntf);
	shd.release(ntf);
	// once we release an acquired object, we should be able to re-acquire it
	BOOST_CHECK(shd.lock_for_commit(ntf));
}

BOOST_AUTO_TEST_SUITE_END()