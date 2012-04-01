#ifndef STM_CPP03_HPP
#define STM_CPP03_HPP

#include <boost/cstdint.hpp>

#if defined(_MSC_VER)
#include <type_traits>
#elif defined(__GNUC__)
#include <tr1/type_traits>
#endif

namespace stm {
	using boost::uint64_t;
  using boost::uint32_t;
	using boost::int16_t;

	namespace detail {
    // C++03 doesn't have rvalue references, so implement a fake move using regular references. 
		template <typename T>
		inline T& move(T& obj) throw() { return obj; }

		using std::tr1::alignment_of;
		using std::tr1::aligned_storage;

    typedef aligned_storage<16, 16>::type max_align_t;
	}
}
#endif
