#ifndef STM_CONFIG_HPP
#define STM_CONFIG_HPP

#define STM_VALIDATE
#define STM_HEADER_ONLY
#define STM_ALLOW_MMX
#define STM_VERSION_WIDTH 64

#include "platform_specific.hpp"

namespace stm {
	namespace backend {
		class list_buffer;
		class array_buffer;
		template <size_t Size>
		class fixed_buffer;
	}

	// keep configuration constants, policies and traits in this namespace to avoid hardcoding them when they're used
	// the important ones can then be factored out as template params later
	namespace config {
		enum {
			// Spinlock max iterationrs when trying to lock a shared during commit
			commit_acquire_retries = 1,
			// how many times should a transaction be allowed to retry before getting blocked (not implemented)	
			// this one is just pointless, isn't it? Only used with age, which we're thinking of ditching
			// also were not blocking but delaying
			// and it doesn't actually happen atm
			retries_before_blocking = 50
		};
		// should the write list generated during commit be sorted
		static const bool sort_write_list = true;

		typedef backend::fixed_buffer<64 * 1024> default_buffer;
		//typedef backend::array_buffer default_buffer;
		//typedef backend::list_buffer default_buffer;

	}
	// type used to store the result when slot index ([0,1)) is extracted from a version field.		
	typedef unsigned int slot_offset_t;
}

#endif
