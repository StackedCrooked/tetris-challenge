#ifndef STM_SHARED_INTERNAL_COMMON_HPP
#define STM_SHARED_INTERNAL_COMMON_HPP

#include "shared_base.hpp"
#include "notifier.hpp"

namespace stm {

	namespace frontend {

		template <typename backend_type, typename T>
		struct shared_internal_common : public backend::shared_base
		{
			typedef T value_type;
			typedef typename std::tr1::remove_const<value_type>::type  non_const_val;

			shared_internal_common& operator= (const shared_internal_common& other) { // TODO: This must be atomic!
				if (this == &other) { return *this; }
				static_cast<backend_type&>(*this)[0] = static_cast<const backend_type&>(other)[0];
				static_cast<backend_type&>(*this)[1] = static_cast<const backend_type&>(other)[1];

				return *this;
			}

			// constness is explicitly removed from return type of rw version, to provoke compiler error on shared<const T>.open_rw() attempts
			template <typename tx_type>
      non_const_val& open_rw(tx_type& tx) { 
        typename tx_type::internal_type& txb = detail::inner_tx(tx);
        typename tx_type::internal_type::notifier_type ntf(txb.group());
				backend::metadata* prev_in_this = txb.find_in_tx(this);
				// If opened before in this transaction, just return existing copy
        if (prev_in_this != NULL) {
					return *backend::get_object<value_type>(prev_in_this);
				}
				else {
					//we need to guarantee that obj won't be modified while we're copying from it. 
					// to avoid this, open_rw has to temporarily acquire
					slot_offset_t slot_id = acquire_for_read(txb.version(), ntf);
          release_guard<typename tx_type::internal_type> grd(txb, *this, slot_id, ntf);
					backend::metadata* prev_in_parent = txb.find_in_parent(this);

					typedef void (*assign_func)(const backend::metadata* psrc, void* pdst);
					assign_func assign = (prev_in_parent != NULL) ? static_cast<assign_func>(nested_assign<value_type>) : static_cast<assign_func>(static_cast<backend_type*>(this)->get_assign_func(slot_id));
					value_type& result = txb.open_rw(static_cast<shared_base&>(*this) // the metadata associated with the object
						, prev_in_parent // pointer to parent copy, if any
						, static_cast<backend_type&>(*this)[slot_id] // the object to copy
					, &destroy<value_type> // the destructor wrapper
						, assign // the assignment operator wrapper
						);
					return result;
				}
			} 
			template <typename tx_type>
      const value_type& open_r(tx_type& tx) {
        typename tx_type::internal_type& txb = detail::inner_tx(tx);
        typename tx_type::internal_type::notifier_type ntf(txb.group());
				backend::metadata* prev = txb.find(this);
        if (prev != NULL) {
          return *backend::get_object<value_type>(prev);
        }        

        // only fails if version is invalid. Throws exception in that case
        slot_offset_t current_active = acquire_for_read(txb.version(), ntf); 
				// register us in buffer immediately, so that if an error occurs from now on, the object gets released when it is popped from buffer
				txb.open_r(*this); 

				// get the value from the slot we got access to
				value_type& result = static_cast<backend_type&>(*this)[current_active];
				return result;
			}

			value_type& active_slot(){
				return static_cast<backend_type&>(*this)[active_slot_id()];
			}

      template <typename tx_internal_type>
			struct release_guard {
				release_guard(tx_internal_type& txb, shared_internal_common& shb, slot_offset_t slot_id, notifier<typename tx_internal_type::manager_t>& ntf) : txb(txb), shb(shb), slot(slot_id), ntf(ntf) {}
				~release_guard() {
					shb.release_reader(slot, ntf);
				} 

				tx_internal_type& txb;
				shared_internal_common& shb;
				slot_offset_t slot;
        notifier<typename tx_internal_type::manager_t>& ntf;
			};
		};
	}
}

#endif
