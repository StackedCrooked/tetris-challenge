#ifndef STM_SHARED_INTERNAL_COMMON_HPP
#define STM_SHARED_INTERNAL_COMMON_HPP

#include "backend.hpp"
#include "buffer_functions.hpp"

namespace stm {
	struct transaction;
	namespace frontend {
		struct transaction_internal;
	}
	namespace detail {
		// helper functions converting between a transaction and its frontend equivalent
		inline transaction& outer_tx(frontend::transaction_internal& tx);
		inline frontend::transaction_internal& inner_tx(transaction& tx);
	}

	namespace frontend {

		template <typename derived, typename T>
		struct shared_internal_common : public backend::shared_base
		{
			typedef T value_type;
			typedef typename std::remove_const<value_type>::type non_const_val;
			
			// constness is explicitly removed from return type of rw version, to provoke compiler error on shared<const T>.open_rw() attempts
			template <typename tx_type>
			non_const_val& open_rw(tx_type& tx) { 
				typename tx_type::internal_type& txb = detail::inner_tx(tx);
				auto& mgr = txb.manager();
				auto lookup_in_current = txb.find_nearest_in_current(this);

				detail::scoped_access_version guard;
				// If opened before in this transaction, just return existing copy
				if (lookup_in_current != mgr.buffer_lookup.end() && lookup_in_current->first == this) {
					return *backend::get_object<value_type>(lookup_in_current->second);
				}
				else {
					backend::metadata* prev_in_parent = txb.find_in_parent(this);

					typedef void (*assign_func)(const backend::metadata* psrc, void* pdst);
					backend::metadata* buffered;

					if (prev_in_parent == NULL){
						slot_offset_t slot_id = acquire_for_read(txb.version()); 
						release_guard<typename tx_type::internal_type> grd(txb, *this, slot_id);

						assign_func assign = static_cast<assign_func>(static_cast<derived*>(this)->get_assign_func(slot_id));

						// allocate in buffer // ought to just return a metadata*
						buffered = txb.open_rw(static_cast<shared_base&>(*this) // the metadata associated with the object
							, static_cast<derived&>(*this)[slot_id] // the object to copy
							, &destroy<value_type> // the destructor wrapper
							, assign // the assignment operator wrapper
							);
					}
					else {
						assign_func assign = static_cast<assign_func>(nested_assign<value_type>);

						// allocate in buffer // ought to just return a metadata*
						buffered = txb.template open_rw_from_parent<T>(static_cast<shared_base&>(*this) // the metadata associated with the object
							, prev_in_parent 
							, &destroy<value_type> // the destructor wrapper
							, assign // the assignment operator wrapper
							);

					}

					mgr.buffer_lookup.insert(lookup_in_current, std::make_pair(this, buffered));
					
					return *backend::get_object<value_type>(buffered);
				}
			} 
			template <typename tx_type>
			const value_type& open_r(tx_type& tx) {
				typename tx_type::internal_type& txb = detail::inner_tx(tx);
				backend::metadata* prev = txb.find(this);
				detail::scoped_access_version guard;
				if (prev != NULL) {
					return *backend::get_object<value_type>(prev);
				}        

				// only fails if version is invalid. Throws exception in that case
				slot_offset_t current_active = acquire_for_read(txb.version()); 
				// register us in buffer immediately, so that if an error occurs from now on, the object gets released when it is popped from buffer
				txb.open_r(*this); 

				// get the value from the slot we got access to
				value_type& result = static_cast<derived&>(*this)[current_active];
				return result;
			}

			value_type& active_slot(){
				detail::scoped_access_version guard;
				return static_cast<derived&>(*this)[active_slot_id()];
			}

			template <typename tx_internal_type>
			struct release_guard {
				release_guard(tx_internal_type& txb, shared_internal_common& shb, slot_offset_t slot_id ) : txb(txb), shb(shb), slot(slot_id) {}
				~release_guard() {
					detail::scoped_access_version guard;
					shb.release_reader(slot);
				} 

				tx_internal_type& txb;
				shared_internal_common& shb;
				slot_offset_t slot;
			};
		};
	}
}

#endif
