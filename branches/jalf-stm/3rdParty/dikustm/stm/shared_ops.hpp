#ifndef STM_SHARED_OPS_HPP
#define STM_SHARED_OPS_HPP

#include "platform_specific.hpp"
#include "config.hpp"

namespace stm {
	namespace detail {
    inline version_field_t get_version(version_field_t hdr) throw() {
			return hdr & ~(static_cast<version_field_t>(1) << ((sizeof(version_field_t) * 8)-1));
		}
		inline slot_offset_t active_offset(version_field_t hdr) throw() {
			return hdr >> (sizeof(version_field_t) * 8 - 1);
		}
		inline version_field_t inactive_offset(version_field_t hdr) throw() {
			return 1 - active_offset(hdr);
		}

		inline version_field_t make_header(slot_offset_t active_slot, version_field_t version) {
			return (static_cast<version_field_t>(active_slot) << (sizeof(version_field_t) * 8 - 1)) | version;
		}

		// a wrapper class for the metadata header (version + active slot id)
		// Centralizes access to these fields, so thye can be easily protected by memory barriers or locking if necessary
		struct header_t {
			header_t() : data() {}
			version_field_t operator()() const throw() { // get header
				version_field_t val = data;
				return val;
			}
			void operator()(version_field_t val) throw() { // set header
				data = val;
			}

		private:
			version_field_t data;
		};
	}
}
#endif
