#ifndef STM_ITERATOR_RW_HPP
#define STM_ITERATOR_RW_HPP

#include <boost/iterator/iterator_facade.hpp>
#include "buffer_entry.hpp"

namespace stm {
	namespace backend {
		// iterator over the list of copies represented by a chain of metadata pointers
		// access to the actual tx-local copy is only possible if the type is known.
		// Then the metadata* can be converted to buffer_entry<...> and its storage field accessed
		struct iterator_rw : public boost::iterator_facade<iterator_rw, shared_base*, boost::forward_traversal_tag>
		{
			iterator_rw() throw() : data(NULL) {}
			iterator_rw(backend::metadata* data) throw() : data(data) {}

			iterator_rw& operator= (iterator_rw other) throw() {
				data = other.data;
				return *this;
			}

		private:
			friend class boost::iterator_core_access;

			void increment() throw() { data = data->tail; } 

			bool equal(const iterator_rw& other) const throw() { return data == other.data;}

			shared_base*& dereference() const throw() { return data->src; }

			backend::metadata* data;
		};
	}
}

#endif
