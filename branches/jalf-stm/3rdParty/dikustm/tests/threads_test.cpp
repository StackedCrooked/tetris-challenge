#include <stm.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <cstdio>

namespace {
	enum { thread_count = 3, iterations = 50000 };

	stm::shared<int> val1(0);
	stm::shared<int> val2(0); 
}

BOOST_AUTO_TEST_SUITE( threads )

bool tx_modify(stm::transaction& tx){
	int& a = val1.open_rw(tx);
	int& b = val2.open_rw(tx);

	--a;
	++b;

	return a + b == 0;
}
struct modify{
	modify(boost::barrier& bar, int& res) : bar(bar), res(res) {}

	void operator()(){
		bar.wait();
		for (int i = 0; i < iterations; ++i){
			res = stm::atomic<bool>(tx_modify) ? 1 : 0 && (res != 0);
		}
	}

	boost::barrier& bar;
	int& res;
};

void verify(stm::transaction& tx) {
	const int& a = val1.open_r(tx);
	const int& b = val2.open_r(tx);

	BOOST_CHECK_EQUAL(-a, thread_count * iterations);
	BOOST_CHECK_EQUAL(b, thread_count * iterations);
}

BOOST_AUTO_TEST_CASE ( short_concurrent_rw_transactions )
{
	boost::barrier bar(thread_count);
	boost::thread_group gr;
	int res[thread_count];
	std::fill(res, res+thread_count, 1);

	boost::system_time start = boost::get_system_time();

	for (int i = 0; i < thread_count; ++i){
		gr.create_thread(modify(bar, res[i]));
	}

	gr.join_all();
	boost::system_time end = boost::get_system_time();

	for (int i = 0; i < thread_count; ++i){
		BOOST_CHECK_EQUAL(res[i], 1);
	}
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(1), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(1), 0);

	stm::atomic(verify);

	boost::int64_t tx_count = static_cast<boost::int64_t>(iterations) * static_cast<boost::int64_t>(thread_count);
	boost::int64_t ms = (end - start).total_milliseconds();

	std::cout << "Total transactions committed: " << tx_count;
	std::cout << "\nTime elapsed: " << ms << " milliseconds";
	std::cout << "\nTransactions/second: " << (tx_count)/ms << "k\n";
}

template <typename T0, typename T1, typename T2, typename T3>
struct modify_mixed {
	modify_mixed(T0& a
		, T1& b
		, T2& c
		, T3& d) : a(a), b(b), c(c), d(d) {}

	bool operator()(stm::transaction& tx){
		const int& x = a.open_r(tx);
		int& y = b.open_rw(tx);
		const int& z = c.open_r(tx);
		int& w = d.open_rw(tx);

		// increment/decrement the two rw's, and check sum of all 4
		++y;
		--w;

		return (x + z == 0) &&  (y + w == 0);
	}

	T0& a;
	T1& b;
	T2& c;
	T3& d;
};

template <typename T0, typename T1, typename T2, typename T3>
modify_mixed<T0, T1, T2, T3> make_tx_mixed(T0& a
																					 , T1& b
																					 , T2& c
																					 , T3& d){
																						 return modify_mixed<T0, T1, T2, T3>(a, b, c, d);
}

template <int iters, typename T0, typename T1, typename T2, typename T3>
struct run_mixed_transactions {
	run_mixed_transactions(boost::barrier& bar
		, int& result
		, T0& a
		, T1& b
		, T2& c
		, T3& d) : bar(bar), result(result), a(a), b(b), c(c), d(d) {}

	void operator()(){
		bar.wait();

		bool res = true;
		for (int i = 0; i < iters; ++i){

			res = res && stm::atomic<bool>(make_tx_mixed(a, b, c, d));
			res = res && stm::atomic<bool>(make_tx_mixed(b, c, d, a));
			res = res && stm::atomic<bool>(make_tx_mixed(c, d, a, b));
			res = res && stm::atomic<bool>(make_tx_mixed(d, a, b, c));
		}
		result = res ? 1 : 0;
		bar.wait();
	}

	boost::barrier& bar;
	int& result;
	T0& a;
	T1& b;
	T2& c;
	T3& d;
};

template <int iters, typename T0, typename T1, typename T2, typename T3>
run_mixed_transactions<iters, T0, T1, T2, T3> make_tx_thread(boost::barrier& bar
																														 , int& result
																														 , T0& a
																														 , T1& b
																														 , T2& c
																														 , T3& d){
																															 return run_mixed_transactions<iters, T0, T1, T2, T3>(bar, result, a, b, c, d);
}

BOOST_AUTO_TEST_CASE ( short_concurrent_r_rw_transactions )
{
	stm::shared<int> val1(0);
	stm::shared<int> val2(0); 
	stm::shared<int> val3(0);
	stm::shared<int> val4(0); 

	boost::barrier bar(thread_count);
	boost::thread_group gr;
	int res[thread_count];

	for (int i = 0; i < thread_count; ++i){
		gr.create_thread(make_tx_thread<iterations>(bar, res[i], val1, val2, val3, val4));
	}

	gr.join_all();

	for (int i = 0; i < thread_count; ++i){
		BOOST_CHECK_NE(res[i], 0);
	}
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(1), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(1), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val3).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val3).lock_val(1), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val4).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val4).lock_val(1), 0);
}

BOOST_AUTO_TEST_CASE ( short_concurrent_mixed_transactions )
{
	int i[2] = {};

	{
		stm::shared<int> val1(0);
		stm::shared<int> val2(0); 
		stm::shared_detached<int> val3(i[0]);
		stm::shared_detached<int> val4(i[1]); 

		boost::barrier bar(thread_count);
		boost::thread_group gr;
		int res[thread_count];

		for (int i = 0; i < thread_count; ++i){
			gr.create_thread(make_tx_thread<iterations>(bar, res[i], val1, val2, val3, val4));
		}

		gr.join_all();

		for (int i = 0; i < thread_count; ++i){
			BOOST_CHECK_NE(res[i], 0);
		}
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(0), 0);
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(1), 0);
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(0), 0);
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(1), 0);
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val3).lock_val(0), 0);
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val3).lock_val(1), 0);
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val4).lock_val(0), 0);
		BOOST_CHECK_EQUAL(stm::detail::open_shared(val4).lock_val(1), 0);
	}

	BOOST_CHECK_EQUAL(i[0], 0);
	BOOST_CHECK_EQUAL(i[1], 0);

}


template <int iters>
struct read_tx_thread {
	read_tx_thread(boost::barrier& bar, stm::shared<int>& val0) : bar(bar), val0(val0) {}

	void operator()(stm::transaction& tx){
		val0.open_r(tx);
	}
	void operator()(){
		bar.wait();

		for (int i = 0; i < iters; ++i){
			stm::atomic(*this);
		}
	}

	boost::barrier& bar;
	stm::shared<int>& val0;
};
BOOST_AUTO_TEST_CASE ( short_concurrent_r_transactions )
{
	stm::shared<int> val1(0);
	stm::shared<int> val2(0); 

	boost::barrier bar(thread_count);
	boost::thread_group gr;

	for (int i = 0; i < thread_count; ++i){
		gr.create_thread(read_tx_thread<iterations*10>(bar, val1));
	}

	gr.join_all();

	BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val1).lock_val(1), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(0), 0);
	BOOST_CHECK_EQUAL(stm::detail::open_shared(val2).lock_val(1), 0);
}

BOOST_AUTO_TEST_SUITE_END()
