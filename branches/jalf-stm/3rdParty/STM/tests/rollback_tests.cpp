#include <stm.hpp>

#include <catch.hpp>
#include <boost/static_assert.hpp>

STM_SILENCE_MMX_WARNING

//namespace {
	enum { iterations = 5 };
	enum rw_policy {
		r,
		w,
		rw
	};

	template <typename T>
	void increment_version(stm::shared<T>& shd, stm::transaction& tx) {
		stm::frontend::shared_internal<int>& backend = stm::detail::open_shared(shd);
		stm::version_field_t ver = stm::detail::inner_tx(tx).version();
		// how many updates? We could do double update (to preserve active slot) when possible, just to be a pain)
		int i = 0;
		for (; i < 2; ++i){
			if (backend.lock_val(1 - backend.active_slot_id()) == 0){
				backend.update_version_and_flip(stm::ver_ops::plus_one(ver));
			}
		}
		assert(i > 0);
	}

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
//}

// todo: I don't really understand all the rollback-on-open ones
TEST_CASE("rollback", "Test rollback of various transactions. For some reason they're all implemented by throwing conflict_exception. Shouldn't they use tx.abort() instead? (and why is tx.rollback() exposed to user at all?")
{ // is it true that throwing conflict_exception causes retry, while .abort() exits and rethrows?

	stm::shared<int> a(0);
	stm::shared<int> b(0);
	stm::shared<int> c(0);

	auto b_a = [&]() { return stm::detail::open_shared(a); };
	auto b_b = [&]() { return stm::detail::open_shared(b); };
	auto b_c = [&]() { return stm::detail::open_shared(c); };

	SECTION("user-rollback", "rollback initiated by the user during a transaction")
	{
		SECTION("reader", "transaction opens for reading, then throws conflict_exception")
		{
			stm::atomic(clean_tx_rollback<r>(a, b));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
		}

		SECTION("writer", "transaction opens for writing, then throws conflict_exception")
		{
			// does this test even make sense? Isn't user supposed to rollback by calling abort() or something? Also why are both tx.rollback and tx.abort exposed? Confusing
			stm::atomic(clean_tx_rollback<w>(a, b));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
		}

		SECTION("readwrite", "transaction opens for writing, and then for reading, finally throws conflict_exception")
		{
			stm::atomic(clean_tx_rollback<rw>(a, b));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
		}
	}

	SECTION("rollback-on-open", "Rollback if validation fails during (re)open")
	{
		SECTION("openr-update-reopenr", "Rollback required if you open for reading twice with an update in between")
		{
			// har altså ikke helt check på hvorfor det er sådan...
			stm::atomic(open_tx_rollback<r, r>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openw-update-reopenr", "Rollback required if you open for writing, then update, then write")
		{
			stm::atomic(open_tx_rollback<w, r>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openrw-update-reopenr", "Rollback required if you open for rw, then update, then write")
		{
			stm::atomic(open_tx_rollback<rw, r>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openr-update-reopenw", "Rollback required if you r-modify-w")
		{
			stm::atomic(open_tx_rollback<r, w>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openw-update-reopenw", "Rollback required if you w-modify-w")
		{
			stm::atomic(open_tx_rollback<w, w>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openrw-update-reopenw", "Rollback required if you rw-modify-w")
		{
			stm::atomic(open_tx_rollback<rw, w>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}
	}

	SECTION("rollback-on-commit", "Rollback if validation fails on commit. Transaction opens, then just before commit, version is updated, invalidating transaction")
	{
		SECTION("openr-openr", "")
		{
			stm::atomic(commit_tx_rollback<r, r>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openw-openr", "")
		{
			stm::atomic(commit_tx_rollback<w, r>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openrw-openr", "")
		{
			stm::atomic(commit_tx_rollback<rw, r>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openr-openw", "")
		{
			stm::atomic(commit_tx_rollback<r, w>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openw-openw", "")
		{
			stm::atomic(commit_tx_rollback<w, w>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}

		SECTION("openrw-openw", "")
		{
			stm::atomic(commit_tx_rollback<rw, w>(a, b, c));

			REQUIRE(b_a().lock_val(0) == 0);
			REQUIRE(b_a().lock_val(1) == 0);
			REQUIRE(b_b().lock_val(0) == 0);
			REQUIRE(b_b().lock_val(1) == 0);
			REQUIRE(b_c().lock_val(0) == 0);
			REQUIRE(b_c().lock_val(1) == 0);
		}
	}
}
