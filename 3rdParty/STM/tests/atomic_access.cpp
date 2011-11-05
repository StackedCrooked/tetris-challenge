#include <stm.hpp>
#include <stm/x86.hpp>

#include <catch.hpp>

TEST_CASE("atomic/64bit", "Verify that writing a 64-bit integer to memory and then reading it back preserves the value, and doesn't thrash other memory")
{
	struct buf_t {
		buf_t() : pre(), mem(), post() {}
		uint64_t pre;
		uint64_t mem;
		uint64_t post;
	} buf;

	stm::atomic_ops::x86::write(static_cast<uint64_t>(0x0123456789abcdefull), &buf.mem);
	REQUIRE(buf.pre == 0ull);
	REQUIRE(buf.post == 0ull);
	REQUIRE(stm::atomic_ops::x86::read(&buf.mem) == 0x0123456789abcdefull);

	REQUIRE(buf.pre == 0ull);
	REQUIRE(buf.post == 0ull);
}
