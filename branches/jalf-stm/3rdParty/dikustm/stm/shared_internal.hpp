#ifndef STM_SHARED_INTERNAL_HPP
#define STM_SHARED_INTERNAL_HPP

#include "shared_internal_common.hpp"
#include "buffer_functions.hpp"

namespace stm {
	namespace frontend {

		template <typename T>
		struct shared_internal : public shared_internal_common<shared_internal<T>, T >
		{
			explicit shared_internal(T val) : val0(val), val1(val) {}
			~shared_internal() {}

			shared_internal(const shared_internal& other) : val0(other.val0), val1(other.val1) {} //todo: make this atomic
			shared_internal& operator= (const shared_internal& other) { //todo: make this atomic
				typedef shared_internal_common<shared_internal<T>, T > base;
				static_cast<base&>(*this) = static_cast<const base&>(other);
				return *this;
			}

			T& operator[](slot_offset_t id) {
				return (id == 0) ? val0 : val1;
			}
			const T& operator[](slot_offset_t id) const {
				return (id == 0) ? val0 : val1;
			}

      // helper function for getting the correct assignment function to use (depending on which slot is currently active)
			typedef void (*assign_func)(const backend::metadata* psrc, void* pdst);
			assign_func get_assign_func(slot_offset_t slot_id) {
				return (slot_id == 0)? static_cast<assign_func>(assign<T, shared_internal<T>, 1>) : static_cast<assign_func>(assign<T, shared_internal<T>, 0>);
			}

		private:
			T val0;
			T val1;
		};
	}
}

#endif
