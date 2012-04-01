#include <stm.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

namespace {
	struct snapshot_func {
		snapshot_func(boost::barrier& bar, int& val0, int& val1, stm::shared<int>& shd0, stm::shared<int>& shd1) : bar(bar), val0(val0), val1(val1), shd0(shd0), shd1(shd1), age() {}
		void operator()(stm::transaction& tx) {
			if (age++ > 0) { // bit of a hack. Should probably define some kind of tx.set_rollback_behavior(abort) function
				tx.abort();
			}
			const int& i0 = shd0.open_r(tx);
			int&       i1 = shd1.open_rw(tx);
			tx.snapshot(&i0, val0);
			tx.snapshot(&i1, val1); // hrm, need to copy i1 before function returns, or it'll go out of scope.
			// unless we can somehow enforce that src must be reference
			bar.wait();
		}

		boost::barrier& bar;
		int& val0;
		int& val1;
		stm::shared<int>& shd0;
		stm::shared<int>& shd1;
		int age;
	};

	struct update_func {
		update_func(boost::barrier& bar, stm::shared<int>& shd) : bar(bar), shd(shd) {}
		void operator()(stm::transaction& tx) {
			shd.open_rw(tx);
		}
		void operator()() {
			stm::atomic(*this);
			bar.wait();
		}

		boost::barrier& bar;
		stm::shared<int>& shd;
	};
}

BOOST_AUTO_TEST_SUITE( snapshot_test )

BOOST_AUTO_TEST_CASE ( snapshot_simple )
{
	stm::shared<int> shd0(0);
	stm::shared<int> shd1(0);
	int res0 = -1;
	int res1 = -1;
	boost::barrier bar(1);
	stm::atomic(snapshot_func(bar, res0, res1, shd0, shd1));
	BOOST_CHECK_EQUAL(res0, 0);
	BOOST_CHECK_EQUAL(res1, 0);
}

BOOST_AUTO_TEST_CASE ( snapshot_fail)
{
	stm::shared<int> shd0(0);
	stm::shared<int> shd1(0);
	int res0 = -1;
	int res1 = -1;
	boost::barrier bar(2);
	boost::thread thr(update_func(bar, shd1));
	BOOST_CHECK_THROW(stm::atomic(snapshot_func(bar, res0, res1, shd0, shd1)), stm::abort_exception);
	thr.join();
	BOOST_CHECK_EQUAL(res0, -1);
	BOOST_CHECK_EQUAL(res1, -1);
}

BOOST_AUTO_TEST_SUITE_END()
