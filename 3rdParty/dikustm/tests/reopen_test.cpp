#include <stm.hpp>
#include <boost/test/unit_test.hpp>

namespace {
	struct tx_func2 {
		tx_func2(stm::shared<int>& val) : val(val) {}
		bool operator()(stm::transaction& tx) {
			bool res = true;
			int& i = val.open_rw(tx);
			++i; // in-buffer version should be 1, global version 0
			int& j = val.open_rw(tx);
			res = res && j == 1;
			++j; // second in-buffer version should be 2, first in-buffer 1 and global 0
			res = res && j == 2;
			res = res && i == 2;

			return res;
		}

		stm::shared<int>& val;
	};

	struct tx_func3 {
		tx_func3(stm::shared<int>& val) : val(val) {}
		bool operator()(stm::transaction& tx) {
			bool res = true;
			int& i = val.open_rw(tx);
			++i; // in-buffer version should be 1, global version 0
			int& j = val.open_rw(tx);
			res = res && j == 1;
			++j; // second in-buffer version should be 2, first in-buffer 1 and global 0
			int& k = val.open_rw(tx);
			++k;
			res = res && k == 3;
			res = res && j == 3;
			res = res && i == 3;

			return res;
		}

		stm::shared<int>& val;
	};

	template <int res>
	struct tx_verify {
		tx_verify(stm::shared<int>& val) : val(val) {}
		bool operator()(stm::transaction& tx) {
			return val.open_r(tx) == res;
		}

		stm::shared<int>& val;
	};
}

BOOST_AUTO_TEST_SUITE( reopen )

BOOST_AUTO_TEST_CASE ( reopen2_test )
{
	stm::shared<int> val(0);
	bool res = stm::atomic<bool>(tx_func2(val));
	BOOST_CHECK(res);
	BOOST_CHECK(stm::atomic<bool>(tx_verify<2>(val)));
}

BOOST_AUTO_TEST_CASE ( reopen3_test )
{
	stm::shared<int> val(0);
	bool res = stm::atomic<bool>(tx_func3(val));
	BOOST_CHECK(res);
	BOOST_CHECK(stm::atomic<bool>(tx_verify<3>(val)));
}

BOOST_AUTO_TEST_SUITE_END()


