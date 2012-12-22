#ifndef H_LIST_BUFFER_FC85379F88F40945BF67A9323314DE14
#define H_LIST_BUFFER_FC85379F88F40945BF67A9323314DE14

#include <deque>
#include "buffer_common.hpp"
#include "utility.hpp"

namespace stm {
	namespace backend {
		class list_buffer {
			typedef std::deque<aux::header_generic*> generic_container;
			typedef std::deque<void*> ptr_container;
			
		public:

			typedef size_t generic_position; // this should really be some opaque iteratorish type
			typedef size_t ptr_position;

			typedef aux::header_generic entry_type;
			
			list_buffer(){}
			list_buffer(list_buffer&& other) : gens(std::move(other.gens)), ptrs(std::move(other.ptrs)){}
			list_buffer& operator=(list_buffer&& other) {
				std::swap(gens, other.gens);
				std::swap(ptrs, other.ptrs);
				return *this;
			}

			template <size_t size, size_t align>
			entry_type* allocate_generic() {
				auto* ptr = aligned_malloc<aux::container_generic<size, align>>();
				gens.push_back(ptr);
				return gens.back();
			}
			
			void* allocate_ptr() {
				ptrs.push_back(NULL);
				return &ptrs.back();
			}

			ptr_position ptr_pos() const { return ptrs.size(); }
			generic_position generic_pos() { return gens.size(); }


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
				std::for_each(gens.begin() + from_pos, gens.end(), [](aux::header_generic* hdr) { 
					aligned_free(hdr);
				});
				gens.erase(gens.begin() + from_pos, gens.end());
			}
			
			bool empty() const {
				return ptrs.empty() && gens.empty();
			}

		private:
			list_buffer(const list_buffer&);
			list_buffer& operator=(const list_buffer&);
			generic_container gens;
			ptr_container ptrs;
		};

		template <>
		struct buffer_traits<list_buffer> {
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
