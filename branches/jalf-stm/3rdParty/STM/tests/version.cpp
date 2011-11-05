#include <stm.hpp>
#include <catch.hpp>
#include <stm/version_ops.hpp>

STM_SILENCE_MMX_WARNING

template <typename V>
struct helper {
	std::vector<unsigned long long> test_versions;
	typedef stm::detail::basic_ver_ops<V> vops;

	helper() {
		test_versions.push_back(0);
		test_versions.push_back(1);
		test_versions.push_back(255);
		test_versions.push_back(256);
		test_versions.push_back(257);
		if (sizeof(V) >= 8) {
			test_versions.push_back((1ull << 32) - 1);
			test_versions.push_back(1ull << 32);
			test_versions.push_back((1ull << 32) + 1);
			test_versions.push_back((1ull << 33) - 2);
			test_versions.push_back((1ull << 33) - 1);
			test_versions.push_back(1ull << 33);
			test_versions.push_back((1ull << 33) + 1);
			test_versions.push_back((1ull << 33) + 2);
		}

	}
	void conversion() {
		for (auto it = test_versions.begin(); it != test_versions.end(); ++it){
			REQUIRE(vops::from_version(vops::to_version(*it)) == *it);
		}
	}

	void increment() {
		for (auto it = test_versions.begin(); it != test_versions.end(); ++it){
			REQUIRE(vops::from_version(vops::plus_one(vops::to_version(*it))) == *it + 1);
		}
	}

	void compare() {
		auto old_ver = vops::to_version(0);
		for (auto it = test_versions.begin(); it != test_versions.end(); ++it){
			auto xplus = vops::plus_one(vops::to_version(*it));
			auto x = vops::to_version(*it);
			//CAPTURE(*it);
			REQUIRE_FALSE(vops::shared_valid_in_tx(vops::set_version_and_flip(old_ver, xplus), x));
			REQUIRE(vops::shared_valid_in_tx(vops::set_version_and_flip(old_ver, x), x));
			REQUIRE(vops::shared_valid_in_tx(vops::set_version_and_flip(old_ver, x), xplus));
		}
	}
};

TEST_CASE("version", "Test version read/write operations")
{
	
	SECTION("correctness", "Verify that version accesses work as expected single-threaded")
	{
		// verify that conversion works, as that's a precondition for all the other tests
		helper<std::uint16_t>().conversion();
		helper<std::uint32_t>().conversion();
		helper<std::uint64_t>().conversion();
#ifndef STM_X64
		helper<__m64>().conversion();
#endif
		
		SECTION("increment", "plus_one(x) == x+1")
		{
		helper<std::uint16_t>().increment();
		helper<std::uint32_t>().increment();
		helper<std::uint64_t>().increment();
#ifndef STM_X64
		helper<__m64>().increment();
#endif
		}

		SECTION("compare", "plus_one(x) is not a valid object version in tx version x")
		{
			
		helper<std::uint16_t>().compare();
		helper<std::uint32_t>().compare();
		helper<std::uint64_t>().compare();
#ifndef STM_X64
		helper<__m64>().compare();
#endif
		}

	}

	SECTION("atomicity", "Verify that version accesses are atomic")
	{

	}

}