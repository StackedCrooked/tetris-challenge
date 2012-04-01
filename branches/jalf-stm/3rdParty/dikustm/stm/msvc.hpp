#ifndef STM_MSVC_HPP
#define STM_MSVC_HPP

#include <intrin.h>
#include "utility.hpp"
#include <Windows.h> // currently only needed for the timer functionality. Should be removed later

#pragma intrinsic (_InterlockedIncrement16)
#pragma intrinsic (_InterlockedIncrement)
#ifdef _WIN64
#pragma intrinsic (_InterlockedIncrement64)
#endif
#pragma intrinsic (_InterlockedCompareExchange64)
#pragma intrinsic (_InterlockedDecrement16)
#pragma intrinsic (_InterlockedCompareExchange16)
#pragma intrinsic (_InterlockedCompareExchange)

namespace stm {
	namespace atomic_ops {
    // convenience wrappers for atomic operations
		inline int16_t increment(int16_t& val) throw() {
			return _InterlockedIncrement16(&val);
		}
		inline long increment(long* val) throw() {
			return _InterlockedIncrement(val);
		}
		inline uint32_t increment(uint32_t& val) throw() {
			return increment(ptr_cast<long*>(&val));
		}
		inline uint64_t increment(uint64_t& val) throw() {
      long long* ptr = ptr_cast<long long*>(&val);
#ifdef _WIN64
			return _InterlockedIncrement64(ptr);
#else
      // x86 doesn't support 64-bit atomic increments, but does have a 64-bit CAS we can use to emulate it
      for (;;) {
        long long expected = *ptr;
        long long initial = _InterlockedCompareExchange64(ptr, expected+1, expected);
        if (initial == expected) { return initial+1; }
      }
#endif
		}
		inline int16_t decrement(int16_t& val) throw() {
			return _InterlockedDecrement16(&val);
		}
  }

  // simple ad-hoc helper class used for benchmarking. Simply returns a millisecond-resolution timestamp
  template <typename type = double>
  struct timestamp {
    timestamp() {
      LARGE_INTEGER tmp;
      QueryPerformanceFrequency(&tmp);
      freq = static_cast<type>(tmp.QuadPart) / static_cast<type>(1000);
    }
    double operator()() {
      LARGE_INTEGER res;
      QueryPerformanceCounter(&res);
      return static_cast<type>(res.QuadPart) / freq;
    }

    type freq;
  };

  // starts a timer when constructed, operator() retrieves time since object was created.
  struct timer {
    timer()  : start(tm()) {}
    double operator()() { return tm() - start; }
  private:
    timestamp<double> tm;
    double start;
  };
}

#endif
