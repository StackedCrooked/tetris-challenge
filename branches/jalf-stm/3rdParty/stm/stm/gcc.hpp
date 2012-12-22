#ifndef STM_GCC_HPP
#define STM_GCC_HPP

#include <stdlib.h>
#include <type_traits>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#define STM_USE_CPP0X
#endif

#ifdef __x86_64__
#define STM_X64
#endif

#define STM_SILENCE_MMX_WARNING

#define STM_BEGIN_SILENCE_MMX_WARNING

#define STM_END_SILENCE_MMX_WARNING

#define STM_TLS __thread

namespace stm {
	inline void* __restrict__ aligned_malloc(size_t size, size_t align) {
		void* result;
		return posix_memalign(&result, align, size) == 0 ? result : NULL; 
	}
	template <typename T>
	inline T* __restrict__ aligned_malloc() {
		return static_cast<T*>(aligned_malloc(sizeof(T), std::alignment_of<T>::value));
	}
	inline void aligned_free(void* ptr) {
		free(ptr);
	}


	namespace atomic_ops {
    // convenience wrappers for atomic operations
		inline int16_t increment(int16_t& val) throw() {
			return __sync_add_and_fetch(&val, 1);
		}
		inline uint32_t increment(uint32_t& val) throw() {
			return __sync_add_and_fetch(&val, 1);
		}
		inline uint64_t increment(uint64_t& val) throw() {
			auto res = __sync_add_and_fetch(&val, 1);
			return res;
		}
		inline int16_t decrement(int16_t& val) throw() {
			auto res = __sync_sub_and_fetch(&val, 1);
#ifdef STM_VALIDATE
			if (res < 0){
				asm("int $3");
			}
#endif
			return res;
		}

		inline void memory_barrier() throw() {
			__sync_synchronize();
		}

		inline void compiler_memory_barrier() throw() {
			__asm__ __volatile__ ("" ::: "memory");
		}

		template <typename T, typename U>
		T cas(T& mem, U newval, U ifequal) throw() {
			return __sync_val_compare_and_swap(&mem, ifequal, newval);
		}
	}
}

#endif
