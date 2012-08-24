#ifndef H_BUFFER_COMMON_B068CAD00D487548B5580E88648A73F1
#define H_BUFFER_COMMON_B068CAD00D487548B5580E88648A73F1

#include <algorithm>

namespace stm {
	namespace backend {
		// fwd declarations
		struct shared_base;
		void update_and_flip(shared_base&, version_field_t commit_version);
		void assign_to_shared(const void*, shared_base*);

		template <typename buffer_type>
		struct buffer_traits;

		namespace aux {
			struct header_generic {
				// calls destructor on the object instance
				destroy_type destroy;
				// calls assignment/move operator to move object from one instance to another
				// to avoid making the decision of which func to call at this time, extend signature to take both ptrs. At insert-time we know if it's outer or not (and have to pass different functions in any case),
				// so at assign-time we don't need to do the check.
				assign_type assign;
				// address of the original object
				// needed for sorting and for determining uniqueness of objects
				shared_base* src;
				// if the object is already open in an outer transaction, this points to that outer object instance
				void* outer;
			};

			template <size_t size, size_t align>
			struct container_generic : header_generic {
				mutable typename std::aligned_storage<size, align>::type object;
			};
		}


	}
}

#endif
