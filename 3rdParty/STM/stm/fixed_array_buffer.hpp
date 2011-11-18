#ifndef STM_FIXED_ARRAY_BUFFER_HPP
#define STM_FIXED_ARRAY_BUFFER_HPP

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
		struct fixed_array_buffer {
			typedef size_t offset_t;
			typedef unsigned char byte_t;
			typedef std::reverse_iterator<shared_base**> iterator_r; // iterator for traversing the list of objects opened for reading
			typedef stm::backend::iterator_rw iterator_rw; // iterator for traversing the list of tx-local copies

			enum { size_in_bytes = stm::config::buffer_size_in_bytes };

			fixed_array_buffer() throw();
			~fixed_array_buffer() throw();

			// attempts to copy an object into the tx-local buffer.
			// throws only if the object's copy ctor does
			// writes the object + associated metadata into the buffer at pos_rw
			// may return NULL to indicate failure (if buffer is full) - this is so we can switch to a fallback linked list buffer or similar when we run out of space
			void* allocate_rw(size_t size, size_t align);

			// register an object opened for reading. Simply push a pointer into the buffer at pos_r
			// may return false to indicate failure (if buffer is full) - this is so we can switch to a fallback linked list buffer or similar when we run out of space
			bool push_r(shared_base* src) throw();

			// releases all acquired read-only objects up until last
			// needs tx version to figure out which slot to decrement
			void release(iterator_r last, version_field_t tx_version) throw();

			void destroy(iterator_rw last) throw();

			// releases all acquired read-only objects up until last
			void pop(iterator_r last) throw();

			// resize buffer, popping off data up until last, without destroying
			void pop(iterator_rw last) throw();

			// iterators should work in reverse insertion order, so we get newest inserts first
			iterator_r begin_r() throw() { return iterator_r(reinterpret_cast<shared_base**>(begin() + pos_r)); }
			iterator_r end_r() throw() { return iterator_r(reinterpret_cast<shared_base**>(begin())); }

			iterator_rw begin_rw() throw() { return iterator_rw(reinterpret_cast<backend::metadata* >(begin()+pos_rw)); }
			iterator_rw end_rw() throw() { return iterator_rw(reinterpret_cast<backend::metadata* >(end())); }

			size_t write_count(iterator_rw last) { return std::distance(begin_rw(), last); }
			size_t read_count(iterator_r last) { return std::distance(begin_r(), last); }
			size_t write_count() { return write_count(end_rw()); }
			size_t read_count() { return read_count(end_r()); }

		private:
			byte_t* begin() throw() { return reinterpret_cast<byte_t*>(&buffer); }
			byte_t* end() throw() { return begin() + size_in_bytes; }

			// the actual buffer space
			detail::aligned_storage<size_in_bytes, detail::alignment_of<detail::max_align_t>::value>::type buffer;

			// offsets into the buffer, indicating where to write next read reference or tx-local copy
			offset_t pos_r; // grows up towards higher addrs - this points to the first address past read list (the address to write the next read to)
			offset_t pos_rw; // grows down towards lower addrs - this points to the last address in the write list (so the next write copy must grow down from here and not use this address)

			template <typename T>
			friend metadata* push_rw(fixed_array_buffer& buf, shared_base* src, backend::metadata* outer_open, const T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst));
		};

		template <typename T>
		metadata* push_rw(fixed_array_buffer& buf, shared_base* src, backend::metadata* outer_open, const T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
			typedef backend::buffer_entry<sizeof(T), detail::alignment_of<T>::value> buffer_entry_type; 
			// write the new object + metadata
			auto old_pos = buf.pos_rw;
			buffer_entry_type* dest_addr = static_cast<buffer_entry_type*>(buf.allocate_rw(sizeof(buffer_entry_type), detail::alignment_of<T>::value));
			new (dest_addr) buffer_entry_type(src, outer_open, obj, reinterpret_cast<backend::metadata*>(buf.begin() + old_pos), destroy, assign);
			// and finally, return the address of the newly allocated buffer entry
			return reinterpret_cast<metadata*>(dest_addr);
		}
	}
}

#endif
