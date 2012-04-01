#include <stm.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/static_assert.hpp>
namespace {

	template <typename T>
	void increment_version(stm::shared<T>& shd, stm::transaction& tx) {
		stm::frontend::shared_internal<int>& backend = stm::detail::open_shared(shd);
		stm::version_field_t ver = stm::detail::inner_tx(tx).version();
		// how many updates? We could do double update (to preserve active slot) when possible, just to be a pain)
		int i = 0;
		for (; i < 2; ++i){
			if (backend.lock_val(1 - backend.active_slot_id()) == 0){
				backend.update_version_and_flip(ver+1);
			}
		}
		assert(i > 0);
	}

	enum { iterations = 5 };

	struct rollback_fixture {
		rollback_fixture() : a(0), b(0), c(0) {}
		stm::shared<int> a;
		stm::shared<int> b;
		stm::shared<int> c;

		stm::frontend::shared_internal<int>& b_a() { return stm::detail::open_shared(a); }
		stm::frontend::shared_internal<int>& b_b() { return stm::detail::open_shared(a); }
		stm::frontend::shared_internal<int>& b_c() { return stm::detail::open_shared(c); }
	};


	enum rw_policy {
		r,
		w,
		rw
	};

	template <rw_policy rw_pol>
	struct clean_tx_rollback {
		clean_tx_rollback(stm::shared<int>& a, stm::shared<int>& b) : a(a), b(b) {}

		void operator()(stm::transaction& tx) {
			if (count++ >= iterations){
				return;
			}

			if (rw_pol != w){
				a.open_r(tx);
			}
			if (rw_pol != r){
				b.open_rw(tx);
			}

			throw stm::conflict_exception();
		}
		stm::shared<int>& a;
		stm::shared<int>& b;
		static int count;
	};
	template <rw_policy rw_pol>
	int clean_tx_rollback<rw_pol>::count;

	template <rw_policy open_pol, rw_policy update_pol>
	struct open_tx_rollback {
		open_tx_rollback(stm::shared<int>& a, stm::shared<int>& b, stm::shared<int>& c) : a(a), b(b), c(c) {}

		void operator()(stm::transaction& tx) {
			BOOST_STATIC_ASSERT((update_pol != rw));
			if (count++ >= iterations){
				return;
			}

			if (open_pol != w){
				a.open_r(tx);
			}
			if (open_pol != r){
				b.open_rw(tx);
			}

			increment_version(c, tx);

			if (update_pol == r){
				c.open_r(tx);
			}
			if (update_pol == w){
				c.open_rw(tx);
			}
		}
		stm::shared<int>& a;
		stm::shared<int>& b;
		stm::shared<int>& c;
		static int count;
	};
	template <rw_policy open_pol, rw_policy update_pol>
	int open_tx_rollback<open_pol, update_pol>::count;

	template <rw_policy open_pol, rw_policy update_pol>
	struct commit_tx_rollback {
		commit_tx_rollback(stm::shared<int>& a, stm::shared<int>& b, stm::shared<int>& c) : a(a), b(b), c(c) {}

		void operator()(stm::transaction& tx) {
			BOOST_STATIC_ASSERT((update_pol != rw));
			if (count++ >= iterations){
				return;
			}

			if (open_pol != w){
				a.open_r(tx);
			}
			if (open_pol != r){
				b.open_rw(tx);
			}

			if (update_pol == r){
				c.open_r(tx);
			}
			if (update_pol == w){
				c.open_rw(tx);
			}

			increment_version(c, tx);
		}
		stm::shared<int>& a;
		stm::shared<int>& b;
		stm::shared<int>& c;
		static int count;
	};
	template <rw_policy open_pol, rw_policy update_pol>
	int commit_tx_rollback<open_pol, update_pol>::count;
}

BOOST_AUTO_TEST_SUITE( rollback )

// attempt rollback outside open/commit operations
BOOST_FIXTURE_TEST_CASE( clean_rollback_r, rollback_fixture )
{
	stm::atomic(clean_tx_rollback<r>(a, b));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( clean_rollback_w, rollback_fixture )
{
	stm::atomic(clean_tx_rollback<w>(a, b));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( clean_rollback, rollback_fixture )
{
	stm::atomic(clean_tx_rollback<rw>(a, b));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
}

// attempt rollback during attempt to open a shared
BOOST_FIXTURE_TEST_CASE( open_rollback_r_r, rollback_fixture )
{
	stm::atomic(open_tx_rollback<r, r>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( open_rollback_w_r, rollback_fixture )
{
	stm::atomic(open_tx_rollback<w, r>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( open_rollback_rw_r, rollback_fixture )
{
	stm::atomic(open_tx_rollback<rw, r>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( open_rollback_r_w, rollback_fixture )
{
	stm::atomic(open_tx_rollback<r, w>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( open_rollback_w_w, rollback_fixture )
{
	stm::atomic(open_tx_rollback<w, w>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( open_rollback_rw_w, rollback_fixture )
{
	stm::atomic(open_tx_rollback<rw, w>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}
// attempt rollback during attempt to commit
BOOST_FIXTURE_TEST_CASE( commit_rollback_r_r, rollback_fixture )
{
	stm::atomic(commit_tx_rollback<r, r>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( commit_rollback_w_r, rollback_fixture )
{
	stm::atomic(commit_tx_rollback<w, r>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( commit_rollback_rw_r, rollback_fixture )
{
	stm::atomic(commit_tx_rollback<rw, r>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( commit_rollback_r_w, rollback_fixture )
{
	stm::atomic(commit_tx_rollback<r, w>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( commit_rollback_w_w, rollback_fixture )
{
	stm::atomic(commit_tx_rollback<w, w>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}

BOOST_FIXTURE_TEST_CASE( commit_rollback_rw_w, rollback_fixture )
{
	stm::atomic(commit_tx_rollback<rw, w>(a, b, c));

	BOOST_CHECK_EQUAL(b_a().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_a().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_b().lock_val(1), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(0), 0);
	BOOST_CHECK_EQUAL(b_c().lock_val(1), 0);
}


BOOST_AUTO_TEST_SUITE_END()