#include <stm.hpp>
#include <boost/test/unit_test.hpp>

namespace {
	struct tx_func {
		tx_func() {}
		void operator()(stm::transaction&) {
			throw std::exception();
		}
	};
}

BOOST_AUTO_TEST_SUITE( abort_tests )

BOOST_AUTO_TEST_CASE ( abort_test )
{
	BOOST_CHECK_THROW(stm::atomic(tx_func()), std::exception);
}

BOOST_AUTO_TEST_SUITE_END()


