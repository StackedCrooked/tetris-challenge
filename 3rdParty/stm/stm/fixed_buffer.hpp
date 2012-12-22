#ifndef H_FIXED_BUFFER_6D2148CAAC72F54D8A3F4AA178D36DD1
#define H_FIXED_BUFFER_6D2148CAAC72F54D8A3F4AA178D36DD1

#include "buffer_common.hpp"
#include "utility.hpp"
#include <deque>
#include <utility>
#include <type_traits>

namespace stm {
	namespace backend {
		template <size_t Size>
		class fixed_buffer {
			enum { size_in_bytes = Size};

		public:
			typedef size_t generic_position;
			typedef size_t ptr_position;

			typedef aux::header_generic entry_type;

			fixed_buffer() : pos_r(0), pos_rw(size_in_bytes) {}
			// could maybe implement move for empty buffers
			fixed_buffer(fixed_buffer&& other) : pos_r(other.pos_r), pos_rw(other.pos_rw) {
				if (!empty()) {
					throw std::exception();
				}
			}

			fixed_buffer& operator= (fixed_buffer&& other) {
				if (!empty() || !other.empty()) {
					throw std::exception();
				}
				return *this;
			}

			template <size_t size, size_t align>
			entry_type* allocate_generic() {
				typedef aux::container_generic<size, align> container;
				auto tmp = (pos_rw - sizeof(container)) & ~static_cast<size_t>(std::alignment_of<container>::value-1);
				if (tmp < pos_r) {
					return NULL;
				}
				pos_rw = tmp;
				auto* res = data() + tmp;
				return reinterpret_cast<entry_type*>(res);
			}
			
			void* allocate_ptr() {
				auto tmp = pos_r + sizeof(void*);
				if (tmp > pos_rw) {
					return NULL;
				}
				auto res = data() + pos_r;
				pos_r = tmp;
				return res;
			}

			ptr_position ptr_pos() const { return pos_r; }
			generic_position generic_pos() const { return pos_rw; }

			template <typename Func>
			bool all(ptr_position first, Func f){
				auto end = reinterpret_cast<void**>(data() + pos_r);
				return std::find_if(reinterpret_cast<void**>(data() + first), end, f) == end;
			}
			template <typename Func>
			void for_each_ptr(ptr_position first, Func f){
				auto end = reinterpret_cast<void**>(data() + pos_r);
				std::for_each(reinterpret_cast<void**>(data() + first), end, f);
			}

			void release_ptrs(ptr_position from_pos) {
				pos_r = from_pos;
			}
			void release_generic(generic_position from_pos) {
				pos_rw = from_pos;
			}

			bool empty() const {
				return pos_r == 0 && pos_rw == size_in_bytes;
			}
		private:
			fixed_buffer(const fixed_buffer&);
			fixed_buffer& operator=(const fixed_buffer&);

			unsigned char* data() {return buffer.array; }
			union {
			typename detail::aligned_storage<size_in_bytes, detail::alignment_of<detail::max_align_t>::value>::type align;
			unsigned char array[size_in_bytes];
			} buffer;

			// offsets into the buffer, indicating where to write next read reference or tx-local copy
			ptr_position pos_r; // grows up towards higher addrs - this points to the first address past read list (the address to write the next read to)
			generic_position pos_rw; // grows down towards lower addrs - this points to the last address in the write list (so the next write copy must grow down from here and not use this address)
		};

		// should really be specialized on container type or something? Hmm. Or not. Maybe specialize on buffer, and for list and array, derive specialization from common container_generic based one
		template <>
		struct buffer_traits<fixed_buffer<64*1024>> {
			typedef aux::header_generic entry_type;

			static void assign(entry_type* obj) { 
				obj->assign(obj, obj->src, obj->outer);
			}
			static void destroy(entry_type* obj){
				obj->destroy(obj);
			}

			template <size_t size, size_t align>
			static void* get_object(const entry_type& node) { return &static_cast<const aux::container_generic<size, align>&>(node).object; }

			template <typename T> 
			static void initialize(entry_type& node, const T& object, destroy_type destroy, assign_type assign, shared_base* src, void* outer) {
				node.assign = assign;
				node.destroy = destroy;
				node.outer = outer;
				node.src = src;
				void* dest = get_object<sizeof(T), std::alignment_of<T>::value>(node);
				new (dest) T(object);
			}
		};

	}
}

#endif

