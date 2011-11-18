#ifndef STM_SHARED_INTERNAL_HPP
#define STM_SHARED_INTERNAL_HPP

#include "shared_internal_common.hpp"
#include "buffer_functions.hpp"
#include <type_traits>

namespace stm {
	namespace frontend {

		template <typename T>
		struct shared_internal : public shared_internal_common<shared_internal<T>, T >
		{
			explicit shared_internal(T val) : val0(val), val1(val) {}
			~shared_internal() {}

			
			template <typename U>
			explicit shared_internal(const shared_internal<U>& other) : val0(other.atomic([&](stm::transaction& tx) { return tx.open_r(other); })), val1(std::move(val0)) {
				// I guess this is a reasonable way to do it:
				// open a transaction to grab from other, store into val0. Then move from that into val1, and manually update version to whatever is current, which also sets val1 as active
				auto version = default_tx_group().get_current_version();
				static_cast<backend::shared_base*>(this)->update_version_and_flip(version);			
			}
			//// why does this exist?
			//template <typename U>
			//shared_internal& operator= (const shared_internal<U>& other) {
			//	shared_internal& self = *this;
			//	stm::atomic([&](stm::transaction& tx) {
			//		tx.open_rw(self) = tx.open_r(other);
			//	});
			//	return *this;
			//}

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
