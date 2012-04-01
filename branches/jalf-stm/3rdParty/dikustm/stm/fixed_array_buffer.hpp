#ifndef STM_FIXED_ARRAY_BUFFER_HPP
#define STM_FIXED_ARRAY_BUFFER_HPP

#include "buffer_ops.hpp"
#include "utility.hpp"
#include "iterator_rw.hpp"
#include "buffer_entry.hpp"
#include "shared_base.hpp"
#include "platform_specific.hpp"

#include <cassert>
#include <boost/utility.hpp>
#include <iterator>

namespace stm {
	namespace backend {
		// stores the local copies of accessed objects. One of these exists per thread, and is shared between all transactions on that thread
		// This also means we don't have to worry about thread safety in this class. Only one thread will access each instance
		template <int size_in_kb = 64>
		struct fixed_array_buffer {
			typedef size_t offset_t;
			typedef unsigned char byte_t;
			typedef std::reverse_iterator<shared_base**> iterator_r; // iterator for traversing the list of objects opened for reading
			typedef stm::backend::iterator_rw iterator_rw; // iterator for traversing the list of tx-local copies

			enum { size_in_bytes = size_in_kb*1024 };

			fixed_array_buffer() throw() : pos_r(0), pos_rw(size_in_bytes) {}
			~fixed_array_buffer() throw() {
				std::for_each(begin_rw(), end_rw(), backend::call_destroy());
			}

			// attempts to copy an object into the tx-local buffer.
			// throws only if the object's copy ctor does
			// writes the object + associated metadata into the buffer at pos_rw
			// may return NULL to indicate failure (if buffer is full) - this is so we can switch to a fallback linked list buffer or similar when we run out of space
			template <typename T>
			T* push_rw(shared_base* src, backend::metadata* outer_open, T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)){
				typedef backend::buffer_entry<sizeof(T), detail::alignment_of<T>::value> buffer_entry_type; // the type of object to place in the buffer

				// move sizeof() bytes towards the beginning of the array, so there's room for writing
				offset_t pos = pos_rw - sizeof(buffer_entry_type); 
				// if this position is not aligned, move a bit further back by masking out the lower bits
				pos &= ~(detail::alignment_of<T>::value-1);

				// if we've passed the read marker or we've wrapped around (because pos is unsigned), we're out of buffer space. Return failure
				if (pos_r > pos || pos > pos_rw) {
					return NULL;
				}

				buffer_entry_type* dest_addr = ptr_cast<buffer_entry_type*>(begin() + pos);

				// write the new object + metadata
				new (dest_addr) buffer_entry_type(src, outer_open, obj, ptr_cast<backend::metadata*>(begin() + pos_rw), destroy, assign);

				pos_rw = pos;

				// and finally, return the address of the newly allocated object
				return ptr_cast<T*>(&dest_addr->storage);
			}

			// register an object opened for reading. Simply push a pointer into the buffer at pos_r
			// may return false to indicate failure (if buffer is full) - this is so we can switch to a fallback linked list buffer or similar when we run out of space
			bool push_r(shared_base* src) throw() {
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
			template <typename notifier_t>
			void release(iterator_r last, version_field_t tx_version, notifier_t& ntf) throw() {
				std::for_each(begin_r(), last, backend::call_release_reader<notifier_t>(tx_version, ntf));
			}

			void destroy(iterator_rw last) throw() {
				std::for_each(begin_rw(), last, backend::call_destroy());
			}
			// releases all acquired read-only objects up until last
			void pop(iterator_r last) throw() {
				if (last == end_r()){ // avoid dereferencing if it's the end iterator
					pos_r = 0;
				}
				else {
					shared_base** last_pos = &*last.base();
					pos_r = ptr_cast<byte_t*>(last_pos) - begin();
				}
			}

			// deletes tx-local copies up until last
			void pop(iterator_rw last) throw() {
				if (last == end_rw()){ // avoid dereferencing if it's the end iterator
					pos_rw = size_in_bytes;
				}
				else {
					shared_base** last_pos = &*last;
					pos_rw = ptr_cast<byte_t*>(last_pos) - begin();
				}
			}

			// iterators should work in reverse insertion order, so we get newest inserts first
			iterator_r begin_r() throw() { return iterator_r(ptr_cast<shared_base**>(begin() + pos_r)); }
			iterator_r end_r() throw() { return iterator_r(ptr_cast<shared_base**>(begin())); }

			iterator_rw begin_rw() throw() { return iterator_rw(ptr_cast<backend::metadata* >(begin()+pos_rw)); }
			iterator_rw end_rw() throw() { return iterator_rw(ptr_cast<backend::metadata* >(end())); }

			size_t write_count(iterator_rw last = end_rw()) { return std::distance(begin_rw(), last); }
			size_t read_count(iterator_r last = end_r()) { return std::distance(begin_r(), last); }

		private:
			byte_t* begin() throw() { return ptr_cast<byte_t*>(&buffer); }
			byte_t* end() throw() { return begin() + size_in_bytes; }

			// the actual buffer space
      typename detail::aligned_storage<size_in_bytes, detail::alignment_of<detail::max_align_t>::value>::type buffer;


			// offsets into the buffer, indicating where to write next read reference or tx-local copy
			offset_t pos_r; // grows up towards higher addrs - this points to the first address past read list (the address to write the next read to)
			offset_t pos_rw; // grows down towards lower addrs - this points to the last address in the write list (so the next write copy must grow down from here and not use this address)

		};
	}
}

#endif
