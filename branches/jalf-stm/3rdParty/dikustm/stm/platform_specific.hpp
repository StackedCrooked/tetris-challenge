#ifndef STM_PLATFORM_SPECIFIC_HPP
#define STM_PLATFORM_SPECIFIC_HPP

// If Visual Studio 2010
#if _MSC_VER > 1500
#define USE_CPP0X
#endif
// if G++ compiling for C++0x
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#define USE_CPP0X
#endif

#ifdef USE_CPP0X
#include "cpp0x.hpp"
#else
#include "cpp03.hpp"
#endif

#if defined(_MSC_VER)
#include "msvc.hpp"
#elif defined(__GNUC__)
#include "gcc.hpp"
#endif

#include <boost/static_assert.hpp>

namespace stm {
	namespace detail {
		BOOST_STATIC_ASSERT((sizeof(size_t) >= sizeof(void*)));
		typedef size_t uintptr_t;
	}
}

#endif
