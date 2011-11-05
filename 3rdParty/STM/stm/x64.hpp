#ifndef H_X64_E98146800D414542BDDBC05D717B14BE
#define H_X64_E98146800D414542BDDBC05D717B14BE

namespace stm {
	namespace atomic_ops {
		namespace x64 {
			template <typename T>
			T read(const T* ptr) throw() {
				return *ptr;
			}

			template <typename T>
			void write(T val, T* ptr) throw() {
				*ptr = val;
			}
		}

		
#ifdef STM_X64
		using namespace x64;
#endif
	}
}

#endif
