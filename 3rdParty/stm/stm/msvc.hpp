#ifndef STM_MSVC_HPP
#define STM_MSVC_HPP

#include <malloc.h>
#include <type_traits>

#if defined(_WIN64) ||defined(_M_X64) || defined(_M_AMD64)
#define STM_X64
#endif

#define STM_SILENCE_MMX_WARNING \
__pragma(warning(disable: 4799))

#define STM_BEGIN_SILENCE_MMX_WARNING \
__pragma(warning(push)) \
__pragma(warning(disable: 4799))

#define STM_END_SILENCE_MMX_WARNING \
__pragma(warning(pop))

#define STM_TLS __declspec(thread)

#include <intrin.h>

#pragma intrinsic (_InterlockedIncrement16)
#pragma intrinsic (_InterlockedIncrement)
#ifdef _WIN64
#pragma intrinsic (_InterlockedIncrement64)
#endif
#pragma intrinsic (_InterlockedDecrement16)
#pragma intrinsic (_InterlockedCompareExchange64)
#pragma intrinsic (_InterlockedCompareExchange16)
#pragma intrinsic (_InterlockedCompareExchange)

namespace stm {
	inline __declspec(noalias) __declspec(restrict) void* aligned_malloc(size_t size, size_t align) {
		return _aligned_malloc(size, align);
	}
	template <typename T>
	inline __declspec(noalias) __declspec(restrict) T* aligned_malloc() {
		return static_cast<T*>(aligned_malloc(sizeof(T), std::alignment_of<T>::value));
	}

	inline void aligned_free(void* ptr) {
		_aligned_free(ptr);
	}

	namespace atomic_ops {
		// convenience wrappers for atomic operations
		inline int16_t increment(int16_t& val) throw() {
			auto res = _InterlockedIncrement16(&val);
			return res;
		}
		inline long increment(long* val) throw() {
			return _InterlockedIncrement(val);
		}
		inline uint32_t increment(uint32_t& val) throw() {
			return static_cast<uint32_t>(increment(reinterpret_cast<long*>(&val)));
		}
		inline uint64_t increment(uint64_t& val) throw() {
			long long* ptr = reinterpret_cast<long long*>(&val);
#ifdef _WIN64
			return static_cast<uint64_t>(_InterlockedIncrement64(ptr));
#else
			// x86 doesn't support 64-bit atomic increments, but does have a 64-bit CAS we can use to emulate it
			for (;;) {
				long long expected = *ptr;
				long long initial = _InterlockedCompareExchange64(ptr, expected+1, expected);
				if (initial == expected) { return static_cast<uint64_t>(initial+1); }
			}
#endif
		}
		inline int16_t decrement(int16_t& val) throw() {
			auto res = _InterlockedDecrement16(&val);
#ifdef STM_VALIDATE
			if (res < 0) {
				__debugbreak();
			}
#endif
			return res;
		}
		namespace aux {
			inline int64_t cas(int64_t* mem, int64_t newval, int64_t ifequal) throw() {
				return static_cast<int64_t>(_InterlockedCompareExchange64(reinterpret_cast<volatile long long*>(mem), static_cast<long long>(newval), static_cast<long long>(ifequal)));
			}
			inline int32_t cas(int32_t* mem, int32_t newval, int32_t ifequal) throw() {
				return static_cast<int32_t>(_InterlockedCompareExchange(reinterpret_cast<volatile long*>(mem), static_cast<long>(newval), static_cast<long>(ifequal)));
			}
			inline int16_t cas(int16_t* mem, int16_t newval, int16_t ifequal) throw() {
				return static_cast<int16_t>(_InterlockedCompareExchange16(reinterpret_cast<volatile short*>(mem), static_cast<short>(newval), static_cast<short>(ifequal)));
			}
		}
		inline int32_t cas(int32_t& mem, int32_t newval, int32_t ifequal) throw() {
			return aux::cas(static_cast<int32_t*>(&mem), static_cast<int32_t>(newval), static_cast<int32_t>(ifequal));
		}
		inline uint32_t cas(uint32_t& mem, uint32_t newval, uint32_t ifequal) throw() {
			return aux::cas(reinterpret_cast<int32_t*>(&mem), static_cast<int32_t>(newval), static_cast<int32_t>(ifequal));
		}

		inline int16_t cas(int16_t& mem, int16_t newval, int16_t ifequal) throw() {
			return aux::cas(static_cast<int16_t*>(&mem), static_cast<int16_t>(newval), static_cast<int16_t>(ifequal));
		}
		inline uint16_t cas(uint16_t& mem, uint16_t newval, uint16_t ifequal) throw() {
			return aux::cas(reinterpret_cast<int16_t*>(&mem), static_cast<int16_t>(newval), static_cast<int16_t>(ifequal));
		}

		inline int64_t cas(int64_t& mem, int64_t newval, int64_t ifequal) throw() {
			return aux::cas(static_cast<int64_t*>(&mem), static_cast<int64_t>(newval), static_cast<int64_t>(ifequal));
		}
		inline uint64_t cas(uint64_t& mem, uint64_t newval, uint64_t ifequal) throw() {
			return aux::cas(reinterpret_cast<int64_t*>(&mem), static_cast<int64_t>(newval), static_cast<int64_t>(ifequal));
		}

		inline void memory_barrier() throw() {
#ifdef STM_X64
			__faststorefence();
#else
			_mm_sfence(); 
#endif
		}

		inline void compiler_memory_barrier() throw() {
			_WriteBarrier();
		}
	}
}

#endif
