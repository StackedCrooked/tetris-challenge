#ifndef STM_SHARED_INTERNAL_DETACHED_HPP
#define STM_SHARED_INTERNAL_DETACHED_HPP

#include "shared_internal_common.hpp"
#include "buffer_functions.hpp"

namespace stm {
	namespace frontend {
		template <typename T>
		struct shared_internal_detached : public shared_internal_common<shared_internal_detached<T>, T >
		{
			shared_internal_detached() : primary(NULL), secondary() {} 
			explicit shared_internal_detached(T& val) : primary(&val), secondary(val) {}
      // "flushes" changes, bringing the attached object up to date before leaving it
			~shared_internal_detached() {
				if (static_cast<backend::shared_base*>(this)->active_slot_id() == 1) { 
					*primary = detail::move(secondary);
				}
			}

			void set_source(T& val) { primary = &val; }

			T& operator[](slot_offset_t id) {
				return (id == 0) ? *primary : secondary;
			}
			const T& operator[](slot_offset_t id) const {
				return (id == 0) ? *primary : secondary;
			}


			typedef void (*assign_func)(const backend::metadata* psrc, void* pdst);
			assign_func get_assign_func(slot_offset_t slot_id) {
				return (slot_id == 0)? static_cast<assign_func>(assign<T, shared_internal_detached<T>, 1>) : static_cast<assign_func>(assign<T, shared_internal_detached<T>, 0>);
			}

		private:
			T* primary;
			T secondary;
		};
	}
}

#endif
