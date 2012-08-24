#ifndef H_ARRAY_BUFFER_269BAC11350DA84FA79FE5CBE67DD4B4
#define H_ARRAY_BUFFER_269BAC11350DA84FA79FE5CBE67DD4B4

#include "buffer_common.hpp"
#include "utility.hpp"
#include <deque>
#include <utility>
#include <type_traits>

namespace stm {
	namespace backend {
		namespace aux {
			template <size_t value>
			struct static_log2;

			template <>
			struct static_log2<1> {
				enum {value = 0 };
			};
			template <>
			struct static_log2<2> {
				enum {value = 1 };
			};
			template <>
			struct static_log2<4> {
				enum {value = 2 };
			};
			template <>
			struct static_log2<8> {
				enum {value = 3 };
			};
			template <>
			struct static_log2<16> {
				enum {value = 4 };
			};
			template <>
			struct static_log2<32> {
				enum {value = 5 };
			};
			template <>
			struct static_log2<64> {
				enum {value = 6 };
			};

			template <typename header_type>
			class custom_deque {

				enum { 
					buffer_align = 32, // just to be on the safe side. 
					max_pages = 16u,
					max_page_bits = aux::static_log2<max_pages>::value,
					min_align_bits = aux::static_log2<std::alignment_of<header_type>::value>::value,
					shift_bits = max_page_bits < min_align_bits ? 0u : max_page_bits - min_align_bits
				};
				static const size_t mask_select_page = (max_pages-1u);
				static const size_t max_page_size = (static_cast<size_t>(-1) & ~mask_select_page) >> shift_bits;

				struct array_triple { // note that pages grow downwards, due to alignment calculation
					unsigned char* page_start;
					unsigned char* page_end;
					unsigned char* last_entry;
				};

			public:
				custom_deque(size_t initial_size) : next_page(0) {
					add_page(initial_size);
				}

				custom_deque(custom_deque&& other) : next_page(other.next_page) {
					for (int i = 0; i < max_pages; ++i) {
						index[i] = other.index[i];
					}
					other.next_page = 0;
				}

				custom_deque& operator=(custom_deque&& other)  {
					std::swap(next_page, other.next_page);
					for (int i = 0; i < max_pages; ++i) {
						std::swap(index[i], other.index[i]);
					}
					return *this;
				}

				~custom_deque() {
					std::for_each(index, index + next_page, [](const array_triple& triple) { aligned_free(triple.page_start); });
				}

				void add_page() {
					add_page((index[next_page - 1].page_end - index[next_page - 1].page_start) * 2u);
				}
				void add_page(size_t size) {
					size = std::min(size, max_page_size);
					auto& page = index[next_page];
					page.page_start = static_cast<unsigned char*>(aligned_malloc(size, buffer_align));
					page.page_end = page.page_start + size;
					page.last_entry = page.page_end;
					++next_page;
				}

				void reset() {
					// delete all pages except the last
					std::for_each(index, index + next_page - 1, [](array_triple& page) { aligned_free(page.page_start); });
					index[0] = index[next_page-1];
					next_page = 1;
				}

				// todo: unused pages could be left allocated in some cases
				header_type* allocate(size_t size, size_t alignment) {
					auto& page = index[next_page-1];
					unsigned char* next_entry = reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr_t>(page.last_entry - size) & ~(alignment - 1));
					if (std::less<unsigned char*>()(next_entry, page.page_start)) { 
						add_page(std::max(size + alignment, static_cast<size_t>((page.page_end - page.page_start) * 2u)));
						return allocate(size, alignment);
					}
					page.last_entry = next_entry;
					return reinterpret_cast<header_type*>(next_entry);
				}

				uintptr_t position() const {
					return to_handle(index[next_page-1].last_entry, next_page - 1);
				}

				void release(uintptr_t pos) {
					auto page = to_page(pos);
					auto ptr = to_ptr(pos, page);
					if (next_page != 1 && ptr == index[0].page_end) {
						reset();
						return;
					}
					std::for_each(index + page + 1, index + next_page, [](const array_triple& trip) { aligned_free(trip.page_start); });
					index[page].last_entry = ptr;
					next_page = page + 1;
				}

				bool empty() const {
					return next_page == 1 && index[0].last_entry == index[0].page_end;
				}

			private:
				unsigned char* to_ptr(uintptr_t handle, size_t page) const {
					auto offset = (handle & ~mask_select_page )>> shift_bits;
					return index[page].page_start + offset;
				}
				size_t to_page(uintptr_t handle) const {
					auto page = handle & mask_select_page;
					return page;
				}
				uintptr_t to_handle(unsigned char* ptr, size_t page) const {
					auto offset = ptr - index[page].page_start;
					auto handle = (offset << shift_bits) | page;
					return handle;
				}

				custom_deque(const custom_deque&);
				custom_deque& operator=(const custom_deque&);
				size_t next_page;
				array_triple index[max_pages];
				// need to set aside some bits for page selector, but depending on alignment, some bits are implicitly 0 and can be reused
			};
		}

		class array_buffer {
			typedef aux::custom_deque<aux::header_generic> generic_container;
			typedef std::deque<void*> ptr_container;
		public:
			typedef uintptr_t generic_position;
			typedef size_t ptr_position;

			typedef aux::header_generic entry_type;

			array_buffer(size_t initial_size = 64*1024) : gens(initial_size) {}
			array_buffer(array_buffer&& other) : gens(std::move(other.gens)), ptrs(std::move(other.ptrs)) {}

			array_buffer& operator=(array_buffer&& other) {
				std::swap(gens, other.gens);
				std::swap(ptrs, other.ptrs);
				return *this;
			}
			template <size_t size, size_t align>
			entry_type* allocate_generic() {
				typedef aux::container_generic<size, align> block_type;
				return gens.allocate(sizeof(block_type), std::alignment_of<block_type>::value);			
			}
			
			void* allocate_ptr() {
				ptrs.push_back(NULL);
				return &ptrs.back();
			}

			ptr_position ptr_pos() const { return ptrs.size(); }
			generic_position generic_pos() const { return gens.position(); }

			template <typename Func>
			bool all(ptr_position first, Func f){
				return std::find_if(ptrs.begin() + first, ptrs.end(), f) == ptrs.end();
			}
			template <typename Func>
			void for_each_ptr(ptr_position first, Func f){
				std::for_each(ptrs.begin() + first, ptrs.end(), f);
			}

			void release_ptrs(ptr_position from_pos) {
				ptrs.erase(ptrs.begin() + from_pos, ptrs.end());
			}
			void release_generic(generic_position from_pos) {
				gens.release(from_pos);
			}

			bool empty() const {
				return ptrs.empty() && gens.empty();
			}
		private:
			array_buffer(const array_buffer&);
			array_buffer& operator=(const array_buffer&);
			generic_container gens;
			ptr_container ptrs;
		};

		// should really be specialized on container type or something? Hmm. Or not. Maybe specialize on buffer, and for list and array, derive specialization from common container_generic based one
		template <>
		struct buffer_traits<array_buffer> {
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
