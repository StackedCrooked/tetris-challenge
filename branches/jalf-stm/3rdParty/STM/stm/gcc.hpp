#ifndef STM_GCC_HPP
#define STM_GCC_HPP

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

		template <typename T, typename U>
		T cas(T& mem, U newval, U ifequal) throw() {
			return __sync_val_compare_and_swap(&mem, ifequal, newval);
		}
	}
}

#endif
