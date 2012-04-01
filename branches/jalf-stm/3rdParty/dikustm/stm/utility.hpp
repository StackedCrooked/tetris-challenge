#ifndef STM_UTILITY_HPP
#define STM_UTILITY_HPP

#include <boost/thread/thread.hpp>

#include <cassert>

namespace stm {
	// casts between pointer types (reinterpret_cast is implementation-defined)
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
}
#endif
