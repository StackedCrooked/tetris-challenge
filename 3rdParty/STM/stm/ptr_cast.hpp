#ifndef H_PTR_CAST_D5DE9B1DA56FBE4792B6F32430CE60FD
#define H_PTR_CAST_D5DE9B1DA56FBE4792B6F32430CE60FD

namespace stm {
	// casts between pointer types (reinterpret_cast is implementation-defined), hope that this is better?
	template <typename To, typename From>
	inline To ptr_cast(From* ptr) throw() {
		return static_cast<To>(static_cast<void*>(ptr));
	}
	template <typename To, typename From>
	inline To ptr_cast(const From* ptr) throw() {
		return static_cast<To>(static_cast<const void*>(ptr));
	}
	template <typename To, typename From>
	inline To ptr_cast(volatile From* ptr) throw() {
		return static_cast<To>(static_cast<volatile void*>(ptr));
	}
	template <typename To, typename From>
	inline To ptr_cast(const volatile From* ptr) throw() {
		return static_cast<To>(static_cast<const volatile void*>(ptr));
	}
}

#endif
