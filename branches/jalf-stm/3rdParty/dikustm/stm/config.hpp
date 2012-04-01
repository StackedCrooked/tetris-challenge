#ifndef STM_CONFIG_HPP
#define STM_CONFIG_HPP

#include "platform_specific.hpp"

namespace stm {

	// keep configuration constants, policies and traits in this namespace to avoid hardcoding them when they're used
	// the important ones can then be factored out as template params later
	namespace config {
		enum {
			// If we fail to acquire a shared during commit, how many times should we retry before rolling back the entire transaction
			commit_acquire_retries = 1,
			open_read_retries = 10,
			// If a transaction has this many writable objects open, we will allocate the sorted write list on the heap
			max_writeset_size_for_stack = 100,
      // how many times should a transaction be allowed to retry before getting blocked (not implemented)
			retries_before_blocking = 50
		};
		// should the write list generated during commit be sorted
    static const bool sort_write_list = true;

	}
  // type used for version field. Toggle this if 64-bit versions are desired
  typedef uint32_t version_field_t;
  // type used to store the result when slot index is extracted from a version field. Should just be a natural int size
  typedef unsigned int slot_offset_t;
}

#endif
