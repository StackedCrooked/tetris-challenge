#ifndef STM_GCC_HPP
#define STM_GCC_HPP

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#define USE_CPP0X
#endif

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
			return __sync_add_and_fetch(&val, 1);
		}
		inline int16_t decrement(int16_t& val) throw() {
			return __sync_sub_and_fetch(&val, 1);
		}
	}
}

#endif
