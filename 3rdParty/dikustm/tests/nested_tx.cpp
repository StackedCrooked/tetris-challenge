#include <stm.hpp>

#include <boost/test/unit_test.hpp>

template <int start_val>
struct inner_tx {
	inner_tx(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		int& i = s.open_rw(tx);
		BOOST_CHECK_EQUAL(i, start_val);
		i += 2;
	}
	stm::shared<int>& s;

	enum { end_val = start_val+2};
};

struct inner_tx_abort {
	inner_tx_abort(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		int& i = s.open_rw(tx);
		BOOST_CHECK_EQUAL(i, 1);
		i += 2;
		tx.abort();
	}
	stm::shared<int>& s;
	enum { end_val = 1};
};

template <typename inner>
struct outer_tx {
	outer_tx(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		int& i = s.open_rw(tx);
		++i;
		BOOST_CHECK_EQUAL(i, 1);
		try {
			stm::atomic(inner(s));
		}
		catch (const stm::abort_exception&){}
		const int expected = inner::end_val;
		BOOST_CHECK_EQUAL(i, expected);
		i+= 4;
	}
	stm::shared<int>& s;
};

template <typename inner>
struct outer_reopen_tx {
	outer_reopen_tx(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		{
			int& i = s.open_rw(tx);
			++i;
			BOOST_CHECK_EQUAL(i, 1);
		}

		stm::atomic(inner(s));

		{
			int& i = s.open_rw(tx);
			BOOST_CHECK_EQUAL(i, 3);
			i+= 4;
		}
	}
	stm::shared<int>& s;
};

template <typename inner>
struct outer_nop {
	outer_nop(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction&) {
		stm::atomic(inner(s));
	}
	stm::shared<int>& s;
};

template <typename inner>
struct outer_post_modify {
	outer_post_modify(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		stm::atomic(inner(s));
		int& i = s.open_rw(tx);
		const int expected = inner::end_val;
		BOOST_CHECK_EQUAL(i, expected);
		++i;
	}
	stm::shared<int>& s;
};

template <typename inner>
struct outer_post_abort {
	outer_post_abort(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		stm::atomic(inner(s));
		tx.abort();
	}
	stm::shared<int>& s;
};

template <int expected>
struct verify {
	verify(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		BOOST_CHECK_EQUAL(s.open_r(tx), expected);
	}
	stm::shared<int>& s;
};

BOOST_AUTO_TEST_SUITE( nested )

BOOST_AUTO_TEST_CASE( nested_test)
{
	stm::shared<int> s(0);
	stm::atomic(outer_tx<inner_tx<1> >(s));
	stm::atomic(verify<7>(s));
}
BOOST_AUTO_TEST_CASE( nested_aborting_test)
{
	stm::shared<int> s(0);
	stm::atomic(outer_tx<inner_tx_abort>(s));
	stm::atomic(verify<5>(s));
}

BOOST_AUTO_TEST_CASE( nested_reopen_test)
{
	stm::shared<int> s(0);
	stm::atomic(outer_reopen_tx<inner_tx<1> >(s));
	stm::atomic(verify<7>(s));
}

BOOST_AUTO_TEST_CASE( inner_open_test)
{
	stm::shared<int> s(0);
	stm::atomic(outer_nop<inner_tx<0> >(s));
	stm::atomic(verify<2>(s));
}

BOOST_AUTO_TEST_CASE( post_modify_test )
{
	stm::shared<int> s(0);
	stm::atomic(outer_post_modify<inner_tx<0> >(s));
	stm::atomic(verify<3>(s));
}
BOOST_AUTO_TEST_CASE( post_abort_test )
{
	stm::shared<int> s(0);
	BOOST_CHECK_THROW(stm::atomic(outer_post_abort<inner_tx<0> >(s)), stm::abort_exception);
	stm::atomic(verify<0>(s));
}
BOOST_AUTO_TEST_SUITE_END()
