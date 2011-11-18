#ifndef H_HEADER_6BF7DA8EEAEF1A40B93871930A693E7E
#define H_HEADER_6BF7DA8EEAEF1A40B93871930A693E7E

#include "utility.hpp"
#include "version_ops.hpp"

namespace stm {
	namespace detail {
		// a wrapper class for the metadata header (version + active slot id)
		// Centralizes access to these fields, so thye can be easily protected by memory barriers or locking if necessary
		struct header_t {
			header_t() : data(ver_ops::zero()) {
				detail::scoped_access_version guard;
			}
			version_field_t operator()() const throw() { // get header
				return atomic_ops::read(&data);
				//version_field_t val = data;
				//return val;
			}
			void operator()(version_field_t val) throw() { // set header
				atomic_ops::write(val, &data);
				//data = val;
			}

#ifndef STM_VALIDATE
		private:
#endif
			version_field_t data;
		};
	}
}

#endif
