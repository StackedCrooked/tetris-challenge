#ifndef STM_BUFFER_FUNCTIONS_HPP
#define STM_BUFFER_FUNCTIONS_HPP

#include "backend.hpp"
#include <type_traits>

namespace stm {
	namespace backend {
		struct shared_base;
	}
	namespace frontend {
		// backend has to store a pointer to a function which calls a tx-local copy's destructor, 
		// but the signature has to be uniform for all object types
		// A pointer to this function satisfies that requirement
		template <typename T, typename buf_traits>
		inline void destroy(const void* obj) throw() {
			const void* ptr = buf_traits::template get_object<sizeof(T), std::alignment_of<T>::value>(*static_cast<const typename buf_traits::entry_type*>(obj));
			static_cast<const T*>(ptr)->T::~T();
		}

		// as above, this function has a general signature that doesn't depend on T, 
		// and will copy the tx-local copy back into a shared handle by calling the (move) assignment operator
		// slot_id is the slot the function will copy *to*
		// shared_type template param should always be shared_internal<T> - (todo: should that be shared_internal_common?)
		// is only templatized to avoid (circular) dependancy on shared_internal
		template <typename T, typename buf_traits, typename shared_type, slot_offset_t slot_id> 
		inline void assign(const void* psrc, backend::shared_base* pdst, void*){
			shared_type* shd = static_cast<shared_type*>(pdst);
			T* dst = &((*shd)[slot_id]);
			const auto& src = *static_cast<const typename buf_traits::entry_type*>(psrc);
			void* object = buf_traits::template get_object<sizeof(T), std::alignment_of<T>::value>(src);
			*dst = detail::move(*static_cast<const T*>(object));
		}

		template <typename Ty, typename buf_traits>
		inline void nested_assign(const void* psrc, backend::shared_base*, void* pdst) {
			const auto& dst = *static_cast<const typename buf_traits::entry_type*>(pdst);
			Ty* dstobj = static_cast<Ty*>(buf_traits::template get_object<sizeof(Ty), std::alignment_of<Ty>::value>(dst));

			const auto& src = *static_cast<const typename buf_traits::entry_type*>(psrc);
			void* object = buf_traits::template get_object<sizeof(Ty), std::alignment_of<Ty>::value>(src);
			*dstobj = detail::move(*static_cast<const Ty*>(object));
		}
	}
}
#endif
