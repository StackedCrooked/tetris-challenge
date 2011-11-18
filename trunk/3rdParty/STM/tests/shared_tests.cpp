
#include <stm/config.hpp>

STM_SILENCE_MMX_WARNING

#include <stm/transaction.hpp>
#include <stm/shared_base.hpp>
#include <stm/shared.hpp>
#include <catch.hpp>


template <typename T>
T setzero() {
	return static_cast<T>(0);
}
#ifndef STM_X64
template <>
__m64 setzero<__m64>() {
	__m64 res = _mm_setzero_si64();
	_mm_empty();
	return res;
}
#endif

stm::version_field_t zero() {
	return setzero<stm::version_field_t>();
}

namespace {
	struct shared_fixture {
		shared_fixture () : shd(42) {}

		stm::frontend::shared_internal<int> shd;
	};
}

TEST_CASE("shared", "")
{
	stm::frontend::shared_internal<int> shd(42);

	SECTION("acquire", "only the first attempt at acquiring for commit should succeed")
	{
		// if no one have acquired it yet, this should succeed
		//REQUIRE(shd.lock_for_commit(zero()));
		auto res = shd.lock_for_commit(zero());
		REQUIRE(res);
		// and then subsequent attempts to acquire should fail
		//REQUIRE_FALSE(shd.lock_for_commit(zero()));
		res = shd.lock_for_commit(zero());
		REQUIRE_FALSE(res);
	}

	SECTION("release", "once we release an acquired object, the next acquire should succeed")
	{
		shd.lock_for_commit(zero());
		shd.release_unchanged();
		// once we release an acquired object, we should be able to re-acquire it
		//REQUIRE(shd.lock_for_commit(zero()));
		auto res = shd.lock_for_commit(zero());
		REQUIRE(res);
	}
}

TEST_CASE("copy_shared", "copy-constructing and assigning shared")
{
	stm::shared<int> sh1(1);

	// copy construct from shared
	stm::shared<int> sh2(sh1);
	
	stm::shared<int> sh3(3);

	// test assignment
	//sh2 = sh3;
}
