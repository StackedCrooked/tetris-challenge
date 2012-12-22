#include <stm.hpp>

#include <set>
#include <string>
#include <catch.hpp>

template <typename buffer_type>
void test_is_empty(buffer_type& buf) {

}
// need to also test after move ctor and move assign

TEST_CASE("buffer", "Test conformance of buffer types")
{
	std::set<std::string> instance_types;
	instance_types.insert(": default-constructed");
	instance_types.insert(": default-move_constructed");
	instance_types.insert(": move_assigned");
	// things to test (for each buffer type, and for each of the above instance types
	// is empty
	// is not empty if add
	// position changes when add (and other position is unaffected)
	// initializing an object doesn't change pos
	// algorithms do not change pos

	for (auto it = instance_types.begin(); it != instance_types.end(); ++it){
		SECTION("default is empty" + *it, "if we haven't added anything, buffer is empty " + *it)
		{
			
		}
	}
}
