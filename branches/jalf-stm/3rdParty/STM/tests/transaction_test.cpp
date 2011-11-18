#include <stm.hpp>
#include <catch.hpp>

STM_SILENCE_MMX_WARNING

#ifndef STM_X64
bool operator == (__m64 lhs, __m64 rhs) {
	bool res = _mm_cvtsi64_si32(_mm_srli_si64(_mm_cmpeq_pi32(lhs, rhs), 16)) == 0xffffffff;
	_mm_empty();
	return res;
}
#endif

TEST_CASE("transaction", "")
{
	stm::shared<int> val(0);

	SECTION("commit-rw", "Committing a modification should set value, swap slots and increment version")
	{
		stm::transaction tx;
		stm::frontend::shared_internal<int>& valb = stm::detail::open_shared(val);
		int old_val = valb.active_slot();

		++val.open_rw(tx);
		stm::frontend::transaction_internal& txb = stm::detail::inner_tx(tx);        
		txb.commit();

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(valb.version() == stm::frontend::default_tx_group().get_current_version());
		REQUIRE(valb.active_slot() == old_val+1);
	}

	SECTION("rollback-rw", "rolling back a modification should not affect value, version or slot")
	{
		stm::transaction tx;
		stm::frontend::shared_internal<int> valb = stm::detail::open_shared(val);
		stm::version_field_t ver = valb.version();
		int old_val = valb.active_slot();

		++val.open_rw(tx);
		REQUIRE_THROWS_AS(tx.abort(), stm::abort_exception);

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(valb.version() == ver);
		REQUIRE(valb.active_slot() == old_val);
	}

	SECTION("commit-r", "committing a read-only transaction shouldn't affect version, value or slot")
	{
		stm::transaction tx;
		stm::frontend::shared_internal<int>& valb = stm::detail::open_shared(val);
		stm::version_field_t ver = valb.version();
		int old_val = valb.active_slot();

		val.open_r(tx);
		stm::frontend::transaction_internal& txb = stm::detail::inner_tx(tx);
		txb.commit();

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(valb.version() == ver);
		REQUIRE(valb.active_slot() == old_val);
	}

	SECTION("rollback-r", "rolling back a read-only transaction shouldn't affect version, value or slot")
	{
		stm::transaction tx;
		stm::frontend::shared_internal<int> valb = stm::detail::open_shared(val);
		stm::version_field_t ver = valb.version();
		int old_val = valb.active_slot();

		val.open_r(tx);
		REQUIRE_THROWS_AS(tx.abort(), stm::abort_exception);

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(valb.version() == ver);
		REQUIRE(valb.active_slot() == old_val);
	}
}
