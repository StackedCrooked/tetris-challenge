#ifndef H_FIXED_ARRAY_BUFFER_IMPL_544E173C39CB614A968EB26480CF5C6E
#define H_FIXED_ARRAY_BUFFER_IMPL_544E173C39CB614A968EB26480CF5C6E

#include "fixed_array_buffer.hpp"
#include "buffer_ops.hpp"
#include "utility.hpp"
#include "iterator_rw.hpp"
#include "buffer_entry.hpp"
#include "shared_base.hpp"
#include "platform_specific.hpp"

#include <cassert>
#include <iterator>

namespace stm {
	namespace backend {
		// stores the local copies of accessed objects. One of these exists per thread, and is shared between all transactions on that thread
		// This also means we don't have to worry about thread safety in this class. Only one thread will access each instance
		STM_LIB_FUNC fixed_array_buffer::fixed_array_buffer() throw() : pos_r(0), pos_rw(size_in_bytes) {}
		STM_LIB_FUNC fixed_array_buffer::~fixed_array_buffer() throw() {
			std::for_each(begin_rw(), end_rw(), backend::call_destroy());
		}

		// maybe refactor so this just allocates space in buffer, but doesn't call ctor. 
		// then it doesn't need to know about destroy/assign either
		STM_LIB_FUNC void* fixed_array_buffer::allocate_rw(size_t size, size_t align) {
			detail::scoped_access_version guard;
			// move sizeof() bytes towards the beginning of the array, so there's room for writing
			offset_t pos = pos_rw - size; 
			// if this position is not aligned, move a bit further back by masking out the lower bits
			pos &= ~(align-1);

			// if we've passed the read marker or we've wrapped around (because pos is unsigned), we're out of buffer space. Return failure
			if (pos_r > pos || pos > pos_rw) {
				return NULL;
			}

			void* dest_addr = static_cast<void*>(begin() + pos);

			pos_rw = pos;

			return dest_addr;
		}

		// register an object opened for reading. Simply push a pointer into the buffer at pos_r
		// may return false to indicate failure (if buffer is full) - this is so we can switch to a fallback linked list buffer or similar when we run out of space
		STM_LIB_FUNC bool fixed_array_buffer::push_r(shared_base* src) throw() {
			// the value type we want to write into the array. Typedef'ed to avoid double pointers because they make my head hurt :(
			typedef shared_base* value_type;
			if (pos_r + sizeof(value_type) > pos_rw){
				return false;
			}

			value_type* next = reinterpret_cast<value_type*>(begin() + pos_r);
			*next = src;
			pos_r += sizeof(value_type);
			return true;
		}

		// releases all acquired read-only objects up until last
		// needs tx version to figure out which slot to decrement
		STM_LIB_FUNC void fixed_array_buffer::release(iterator_r last, version_field_t tx_version) throw() {
			std::for_each(begin_r(), last, backend::call_release_reader(tx_version));
		}

		// call destructors without popping
		STM_LIB_FUNC void fixed_array_buffer::destroy(iterator_rw last) throw() {
			{ 
				detail::scoped_access_version guard;
			}
			std::for_each(begin_rw(), last, backend::call_destroy());
		}
		// releases all acquired read-only objects up until last
		STM_LIB_FUNC void fixed_array_buffer::pop(iterator_r last) throw() {
			if (last == end_r()){ // avoid dereferencing if it's the end iterator
				pos_r = 0;
			}
			else {
				shared_base** last_pos = &*last.base();
				pos_r = reinterpret_cast<byte_t*>(last_pos) - begin();
			}
		}

		// resize buffer, popping off data up until last
		STM_LIB_FUNC void fixed_array_buffer::pop(iterator_rw last) throw() {
			if (last == end_rw()){ // avoid dereferencing if it's the end iterator
				pos_rw = size_in_bytes;
			}
			else {
				shared_base** last_pos = &*last;
				pos_rw = reinterpret_cast<byte_t*>(last_pos) - begin();
			}
		}
	}
}

#endif
