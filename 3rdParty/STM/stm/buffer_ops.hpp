#ifndef STM_BUFFER_OPS_HPP
#define STM_BUFFER_OPS_HPP

#include "shared_base.hpp"
#include "buffer_entry.hpp"

namespace stm {
	namespace backend {
		// helper functor to simplify calling destroy with std algorithms - could be replaced by c++0x lambda when supported
		struct call_destroy { 
			void operator()(shared_base*& src) throw();
		};
		// helper functor as above
		struct call_rollback_release { 
			void operator()(std::pair<shared_base*, metadata*> src) throw();

			version_field_t commit_version;
		};
		struct call_commit_release { 
			call_commit_release(version_field_t commit_version) throw();
			void operator()(std::pair<shared_base*, metadata*> src) throw();

			version_field_t commit_version;
		};
		// helper functor as above
		struct call_release_reader { 
			call_release_reader(version_field_t tx_version) throw();
			void operator()(shared_base*& src) throw();

			version_field_t tx_version;
		};
	}
}

#endif
