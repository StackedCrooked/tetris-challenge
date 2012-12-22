#include <stm.hpp>

#include <catch.hpp>

template <int start_val>
struct inner_tx {
	inner_tx(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		int& i = s.open_rw(tx);
		REQUIRE(i == start_val);
		i += 2;
	}
	stm::shared<int>& s;

	enum { end_val = start_val+2};
};

struct inner_tx_abort {
	inner_tx_abort(stm::shared<int>& s) : s(s) {}
	void operator()(stm::transaction& tx) {
		int& i = s.open_rw(tx);
		REQUIRE(i == 1);
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
		REQUIRE(i == 1);
		try {
			stm::atomic(inner(s));
		}
		catch (const stm::abort_exception&){}
		const int expected = inner::end_val;
		REQUIRE(i == expected);
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
			REQUIRE(i == 1);
		}

		stm::atomic(inner(s));

		{
			int& i = s.open_rw(tx);
			REQUIRE(i == 3);
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
		REQUIRE(i == expected);
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
		REQUIRE(s.open_r(tx) == expected);
	}
	stm::shared<int>& s;
};

TEST_CASE("nested", "All tests of nested transactions")
{// todo: ugh, need to replace these functors, impossible to guess what they do
	SECTION("nested-nop", "Empty nested transactions commits ok")
	{
		stm::atomic([](stm::transaction&){
			stm::atomic([](stm::transaction&) {
			});
		});
	}
	SECTION("general", "Basic nested transaction commits ok")
	{
		stm::shared<int> s(0);
		stm::atomic(outer_tx<inner_tx<1> >(s));
		stm::atomic(verify<7>(s));
	}
	SECTION("handling-inner-abort", "When inner tx aborts, outer can handle it to prevent complete rollback")
	{
		stm::shared<int> s(0);
		stm::atomic(outer_tx<inner_tx_abort>(s));
		stm::atomic(verify<5>(s));
	}
	SECTION("reopen", "outer reopens object after inner commits")
	{
		stm::shared<int> s(0);
		stm::atomic(outer_reopen_tx<inner_tx<1> >(s));
		stm::atomic(verify<7>(s));
	}
	SECTION("inner-opnes", "outer is nop, inner opens object")
	{
		stm::shared<int> s(0);
		stm::atomic(outer_nop<inner_tx<0> >(s));
		stm::atomic(verify<2>(s));
	}
	SECTION("modify-after-inner", "outer modifies object after inner committed")
	{
		stm::shared<int> s(0);
		stm::atomic(outer_post_modify<inner_tx<0> >(s));
		stm::atomic(verify<3>(s));
	}
	SECTION("abort-after-inner", "outer aborts after inner committed")
	{
		stm::shared<int> s(0);
		REQUIRE_THROWS_AS(stm::atomic(outer_post_abort<inner_tx<0> >(s)), stm::abort_exception);
		stm::atomic(verify<0>(s));
	}

}

TEST_CASE("nested/inner-opens", "outer is nop, inner opens object")
{
	stm::shared<int> s(0);
	stm::atomic(outer_nop<inner_tx<0> >(s));
	stm::atomic(verify<2>(s));
}


TEST_CASE("nested/ultimate", "mainly for debugging")
{
	stm::shared<int> so[] = { stm::shared<int>(0), stm::shared<int>(0), stm::shared<int>(0) };

	CAPTURE(&so[0]);
	CAPTURE(&so[1]);
	CAPTURE(&so[2]);
	stm::atomic([&so](stm::transaction& txo){
		decltype(so)& si = so;

		int& s0 = so[0].open_rw(txo);
		s0 = 1;
		CAPTURE((void*)&s0);
		int& s1 = so[1].open_rw(txo); 
		s1 = 2;
		CAPTURE((void*)&s1);
		stm::atomic([&si, &txo](stm::transaction& txi){
			int& s1 = si[1].open_rw(txi);
			REQUIRE(s1 == 2);
			s1 = 3;
			CAPTURE((void*)&s1);
			int& s2 = si[2].open_rw(txi);
			REQUIRE(s2 == 0);
			s2 = 4;
			CAPTURE((void*)&s2);
		});

		int& s2 = si[2].open_rw(txo);
		REQUIRE(s2 == 4);
		s2 = 5;
		CAPTURE((void*)&s2);
	});
}
	
