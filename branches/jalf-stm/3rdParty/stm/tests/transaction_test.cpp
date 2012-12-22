#include <stm.hpp>
#include <catch.hpp>

STM_SILENCE_MMX_WARNING

#ifndef STM_X64
bool eq(__m64 lhs, __m64 rhs) {
	bool res = static_cast<unsigned int>(_mm_cvtsi64_si32(_mm_srli_si64(_mm_cmpeq_pi32(lhs, rhs), 16))) == 0xffffffffu;
	_mm_empty();
	return res;
}
#else
	bool eq(stm::version_field_t lhs, stm::version_field_t rhs) { return lhs == rhs; }
#endif

auto& mgr = stm::frontend::get_manager();
	
TEST_CASE("transaction", "")
{
	stm::shared<int> val(0);
	SECTION("commit-rw", "Committing a modification should set value, swap slots and increment version")
	{
		stm::frontend::transaction_internal txb(mgr, mgr.group().get_current_version());
		stm::transaction tx(txb);
		stm::frontend::shared_internal<int>& valb = stm::detail::open_shared(val);
		int old_val = valb.active_slot();

		++val.open_rw(tx);
		txb.commit();

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(eq(valb.version(), stm::frontend::default_tx_group().get_current_version()));
		REQUIRE(valb.active_slot() == old_val+1);
	}

	SECTION("rollback-rw", "rolling back a modification should not affect value, version or slot")
	{
		stm::frontend::transaction_internal txb(mgr, mgr.group().get_current_version());
		stm::transaction tx(txb);
		stm::frontend::shared_internal<int> valb = stm::detail::open_shared(val);
		stm::version_field_t ver = valb.version();
		int old_val = valb.active_slot();

		++val.open_rw(tx);
		REQUIRE_THROWS_AS(tx.abort(), stm::abort_exception);

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(eq(valb.version(), ver));
		REQUIRE(valb.active_slot() == old_val);
	}

	SECTION("commit-r", "committing a read-only transaction shouldn't affect version, value or slot")
	{
		stm::frontend::transaction_internal txb(mgr, mgr.group().get_current_version());
		stm::transaction tx(txb);
		stm::frontend::shared_internal<int>& valb = stm::detail::open_shared(val);
		stm::version_field_t ver = valb.version();
		int old_val = valb.active_slot();

		val.open_r(tx);
		txb.commit();

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(eq(valb.version(), ver));
		REQUIRE(valb.active_slot() == old_val);
	}

	SECTION("rollback-r", "rolling back a read-only transaction shouldn't affect version, value or slot")
	{
		stm::frontend::transaction_internal txb(mgr, mgr.group().get_current_version());
		stm::transaction tx(txb);
		stm::frontend::shared_internal<int> valb = stm::detail::open_shared(val);
		stm::version_field_t ver = valb.version();
		int old_val = valb.active_slot();

		val.open_r(tx);
		REQUIRE_THROWS_AS(tx.abort(), stm::abort_exception);

		REQUIRE(valb.lock_val(0) == 0);
		REQUIRE(valb.lock_val(1) == 0);
		REQUIRE(eq(valb.version(), ver));
		REQUIRE(valb.active_slot() == old_val);
	}
}
