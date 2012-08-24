#ifndef STM_CPP0X_HPP
#define STM_CPP0X_HPP

#include <stdint.h>
#include <type_traits>
#include <utility>

namespace stm {
    using ::uint64_t;
    using ::uint32_t;
	using ::int16_t;

	namespace detail {
		using std::move;

		using std::alignment_of;
		using std::aligned_storage;

    // Should be replaced with using std::max_align_t, but this type is not yet implemented in major compilers
    typedef aligned_storage<16, 16>::type max_align_t;

    // dummy function for allowing decltype to deduce the return type of atomic
    template <class T>
    T fake();
  }
}
#endif
