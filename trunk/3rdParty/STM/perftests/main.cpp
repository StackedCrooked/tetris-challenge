#ifdef __GNUC__
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE perf

#include <boost/test/unit_test.hpp>

namespace {
	struct sanity_check {};

}

BOOST_FIXTURE_TEST_CASE( hello_fixture, sanity_check )
{
}
BOOST_AUTO_TEST_CASE( hello_test_world )
{
}
