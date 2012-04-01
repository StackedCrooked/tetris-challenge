#include <stm.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

namespace {
	struct tx_read {
		tx_read(stm::shared<int>& x, stm::shared<int>& y) : x(x), y(y) {}
		bool operator()(stm::transaction& tx) {
			int& i = x.open_rw(tx);
			int& j = y.open_rw(tx);

			return i+j == 0;
		}

		stm::shared<int>& x;
		stm::shared<int>& y;
	};

	struct tx_write {
		tx_write(stm::shared<int>& x, stm::shared<int>& y) : x(x), y(y) {}
		bool operator()(stm::transaction& tx) {
			int& i = x.open_rw(tx);
			int& j = y.open_rw(tx);

			++i;
			--j;

			return i+j == 0;
		}

		stm::shared<int>& x;
		stm::shared<int>& y;
	};

	template <int iterations>
	struct reader {
		reader(boost::barrier& bar, int& result, stm::shared<int>& x, stm::shared<int>& y) : bar(bar), result(result), x(x), y(y) {}
		void operator()() {
			bar.wait();
			bool res = true;
			for (int i = 0; i < iterations; ++i){
				res = res && stm::atomic<bool>(tx_read(x, y));
			}
			result = res ? 1 : 0;
		}

		boost::barrier& bar;
		int& result;
		stm::shared<int>& x;
		stm::shared<int>& y;
	};

	template <int iterations>
	struct writer {
		writer(boost::barrier& bar, int& result, stm::shared<int>& x, stm::shared<int>& y) : bar(bar), result(result), x(x), y(y) {}
		void operator()() {
			bar.wait();
			bool res = true;
			for (int i = 0; i < iterations; ++i){
				res = res && stm::atomic<bool>(tx_write(x, y));
			}
			result = res ? 1 : 0;
		}

		boost::barrier& bar;
		int& result;
		stm::shared<int>& x;
		stm::shared<int>& y;
	};

	template <int readers, int writers, int iterations>
	void run_tests(int (&res)[readers+writers]){
		stm::shared<int> x(0);
		stm::shared<int> y(0);

		boost::barrier bar(readers+writers);
		boost::thread_group gr;

		for (int i = 0; i < writers; ++i){
			gr.create_thread(writer<iterations>(bar, res[readers+i], x, y));
		}
		for (int i = 0; i < readers; ++i){
			gr.create_thread(reader<iterations>(bar, res[i], x, y));
		}

		gr.join_all();
	}

	enum { iterations = 50000 };
}

BOOST_AUTO_TEST_SUITE( rw_tests )

BOOST_AUTO_TEST_CASE( one_writer_one_reader )
{
	enum { readers = 1, writers = 1 };
	int res[readers+writers];
	run_tests<readers, writers, iterations>(res);

	for (int i = 0; i < readers+writers; ++i) {
		BOOST_CHECK_NE(res[i], 0);
	}
}

BOOST_AUTO_TEST_CASE( one_writer_n_reader )
{
	enum { readers = 2, writers = 1 };
	int res[readers+writers];
	run_tests<readers, writers, iterations>(res);

	for (int i = 0; i < readers+writers; ++i) {
		BOOST_CHECK_NE(res[i], 0);
	}
}

BOOST_AUTO_TEST_CASE( n_writer_n_reader )
{
	enum { readers = 2, writers = 2 };
	int res[readers+writers];
	run_tests<readers, writers, iterations>(res);

	for (int i = 0; i < readers+writers; ++i) {
		BOOST_CHECK_NE(res[i], 0);
	}
}

BOOST_AUTO_TEST_SUITE_END()
