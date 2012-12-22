#ifndef STM_UTILITY_HPP
#define STM_UTILITY_HPP

#include <boost/thread/thread.hpp>
#include "config.hpp"

#include "scope_guard.hpp"

#ifdef STM_X64
#include "x64.hpp"
#else
#include "x86.hpp"
#endif

#if defined(STM_HEADER_ONLY)
#define STM_LIB_FUNC inline
#else
#define STM_LIB_FUNC
#endif


namespace stm {
#if STM_VERSION_WIDTH == 64
#if defined(STM_X64) || !defined(STM_ALLOW_MMX)
	typedef uint64_t version_field_t;
#else
	typedef __m64 version_field_t;
#define STM_USE_MMX
#endif
#elif STM_VERSION_WIDTH == 32
	typedef uint32_t version_field_t;
#elif STM_VERSION_WIDTH == 16
	typedef uint16_t version_field_t;
#elif STM_VERSION_WIDTH == 8
	typedef uint8_t version_field_t;
#endif

	inline void yield() throw() {
		boost::this_thread::yield();
	}
	inline void sleep(int ms) throw() {
		boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
	}

  // simple hack: intended to gradually block the calling thread for longer and longer periods to reduce contention
	inline void backoff(int attempt) throw() {
		if (attempt > 100){
			yield();
		}
	}

	inline void delay() throw() {
#if defined(__GNUC__)
		__asm__ __volatile__ ("pause");
#elif defined(_MSC_VER)
		_mm_pause();
#endif
	}

	namespace backend {
		struct shared_base;
	}
	typedef void (*assign_type)(const void*, backend::shared_base*, void*);
	typedef void (*destroy_type)(const void*);
}
#endif
