#ifndef STM_BUFFER_FUNCTIONS_HPP
#define STM_BUFFER_FUNCTIONS_HPP

#include "buffer_entry.hpp" // defines metadata

namespace stm {
	namespace frontend {
		// backend has to store a pointer to a function which calls a tx-local copy's destructor, 
		// but the signature has to be uniform for all object types
		// A pointer to this function satisfies that requirement
		template <typename T>
		inline void destroy(backend::metadata* ptr) throw() {
			typedef backend::buffer_entry<sizeof(T), detail::alignment_of<T>::value> local_type;
			local_type* local = ptr_cast<local_type*>(ptr);
			T* obj = ptr_cast<T*>(&local->storage);
			obj->T::~T();
		}

		// as above, this function has a general signature that doesn't depend on T, 
		// and will copy the tx-local copy back into a shared handle by calling the (move) assignment operator
		// slot_id is the slot the function will copy *to*
		// shared_type template param should always be shared_internal<T> - 
		// is only templatized to avoid (circular) dependancy on shared_internal
		template <typename T, typename shared_type, slot_offset_t slot_id> 
		inline void assign(const backend::metadata* psrc, void* pdst){
			typedef backend::buffer_entry<sizeof(T), detail::alignment_of<T>::value> local_type;
			const local_type* local = ptr_cast<const local_type*>(psrc);
			const T* src = ptr_cast<const T*>(&local->storage);

			shared_type* shd = static_cast<shared_type*>(pdst);
			T* dst = &((*shd)[slot_id]);
			*dst = detail::move(*src);
		}


		template <typename T>
		inline void nested_assign(const backend::metadata* psrc, void* pdst) {
			typedef backend::buffer_entry<sizeof(T), detail::alignment_of<T>::value> local_type;
			const local_type* local = ptr_cast<const local_type*>(psrc);
			const T* src = ptr_cast<const T*>(&local->storage);

			local_type* dest = ptr_cast<local_type*>(pdst);
			*ptr_cast<T*>(&dest->storage) = detail::move(*src);
		}
	}
}
#endif
