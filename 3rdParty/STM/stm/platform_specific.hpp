#ifndef STM_PLATFORM_SPECIFIC_HPP
#define STM_PLATFORM_SPECIFIC_HPP

#if _MSC_VER < 1600 && !defined(__GXX_EXPERIMENTAL_CXX0X__)
#error C++03 support has been removed!
#endif

#include "cpp0x.hpp"

#if defined(_MSC_VER)
#include "msvc.hpp"
#elif defined(__GNUC__)
#include "gcc.hpp"
#endif

#endif
