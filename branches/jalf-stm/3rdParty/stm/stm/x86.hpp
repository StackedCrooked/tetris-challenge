#ifndef H_X86_7A2E439F801B7C4893B4D5394B31F8D0
#define H_X86_7A2E439F801B7C4893B4D5394B31F8D0

#include <emmintrin.h>

namespace stm {
	namespace atomic_ops {
		namespace x86 {
			template <typename T>
			inline T read(const T* ptr) throw() {
				return *ptr;
			}

			template <typename T>
			inline void write(T val, T* ptr) throw() {
				*ptr = val;
			}

#ifndef STM_X64
STM_BEGIN_SILENCE_MMX_WARNING

			template <>
			inline uint64_t read(const uint64_t* ptr) throw() {
				union {
					__m64 vec;
					uint64_t scalar;
				} u;
				u.vec = *reinterpret_cast<const __m64*>(ptr);
				_mm_empty();
				return u.scalar;
			}
			template <>
			inline void write(uint64_t val, uint64_t* ptr) throw() {
				union {
					__m64 vec;
					uint64_t scalar;
				} u;
				u.scalar = val;
				*reinterpret_cast<__m64*>(ptr) = u.vec;
				_mm_empty();
			}

STM_END_SILENCE_MMX_WARNING
#endif
		}

#ifndef STM_X64
		using namespace x86;
#endif
	}
}

#ifndef STM_X64
namespace std {
	template <>
	inline void swap<__m64>(__m64& lhs, __m64& rhs) {
		__m64 tmp = lhs;
		lhs = rhs;
		rhs = tmp;
		_mm_empty();
	}
}
#endif

#endif
