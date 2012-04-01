#ifndef STM_BUFFER_OPS_HPP
#define STM_BUFFER_OPS_HPP

#include "shared_base.hpp"
#include "buffer_entry.hpp"

namespace stm {
	namespace backend {
		// helper functor to simplify calling destroy with std algorithms - could be replaced by c++0x lambda when supported
		struct call_destroy { 
			void operator()(shared_base*& src) throw() {
				metadata* meta = ptr_cast<metadata*>(&src);
				meta->destroy(meta);
			}
		};
		// helper functor as above
		template <typename notifier_t>
		struct call_release { 
			call_release(notifier_t& ntf) : ntf(ntf) {}
			void operator()(shared_base** src) throw() {
				(*src)->release(ntf);
			}

			notifier_t& ntf;
		};
		// helper functor as above
		template <typename notifier_t>
		struct call_release_reader { 
			call_release_reader(version_field_t tx_version, notifier_t& ntf) throw() : tx_version(tx_version), ntf(ntf) {}
			void operator()(shared_base*& src) throw() {
				slot_offset_t locked_slot = src->get_locked_slot(tx_version);
				src->release_reader(locked_slot, ntf);
			}

			version_field_t tx_version;
			notifier_t& ntf;
		};
	}
}

#endif
