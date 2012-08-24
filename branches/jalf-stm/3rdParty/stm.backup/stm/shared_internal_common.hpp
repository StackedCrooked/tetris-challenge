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
				typedef typename tx_type::internal_type::buffer_type buffer_type;
				typedef backend::buffer_traits<buffer_type> buf_traits;

				typename tx_type::internal_type& txb = detail::inner_tx(tx);
				auto& mgr = txb.manager();
				auto lookup_in_current = txb.find_nearest_in_current(this);

				detail::scoped_access_version guard;
				// If opened before in this transaction, just return existing copy
				if (lookup_in_current != mgr.buffer_lookup.end() && lookup_in_current->first == this) {
					const auto& entry = *static_cast<typename buf_traits::entry_type*>(lookup_in_current->second);
					return *static_cast<T*>(backend::buffer_traits<buffer_type>::template get_object<sizeof(T), std::alignment_of<T>::value>(entry));
				}
				else {
					void* prev_in_parent = txb.find_in_parent(this);

					typename backend::buffer_traits<buffer_type>::entry_type* buffered;

					if (prev_in_parent == NULL){
						slot_offset_t slot_id = acquire_for_read(txb.version()); 
						auto& self = *this;
						auto grd = scope_guard([&self, slot_id](){
							detail::scoped_access_version guard;
							self.release_reader(slot_id);
						});

						assign_type assign = static_cast<assign_type>(static_cast<derived*>(this)->template get_assign_func<buf_traits>(slot_id));

						// allocate in buffer // ought to just return a metadata*
						buffered = txb.open_rw(static_cast<shared_base&>(*this) // the metadata associated with the object
							, static_cast<derived&>(*this)[slot_id] // the object to copy
							, &destroy<value_type, buf_traits> // the destructor wrapper
							, assign // the assignment operator wrapper
							);
					}
					else {
						assign_type assign = static_cast<assign_type>(nested_assign<value_type, buf_traits>);

						// allocate in buffer // ought to just return a metadata*
						buffered = txb.template open_rw_from_parent<T>(static_cast<shared_base&>(*this) // the metadata associated with the object
							, prev_in_parent 
							, &destroy<value_type, buf_traits> // the destructor wrapper
							, assign // the assignment operator wrapper
							);

					}

					mgr.buffer_lookup.insert(lookup_in_current, std::make_pair(this, buffered));
					
					
					return *static_cast<T*>(backend::buffer_traits<buffer_type>::template get_object<sizeof(T), std::alignment_of<T>::value>(*buffered));
					//return *static_cast<value_type*>(buffered);
				}
			} 
			template <typename tx_type>
			const value_type& open_r(tx_type& tx) const {
				typename tx_type::internal_type& txb = detail::inner_tx(tx);
				void* prev = txb.find(this);
				detail::scoped_access_version guard;
				if (prev != NULL) {
                    typedef typename tx_type::internal_type::buffer_type buffer_type;
                    typedef typename backend::buffer_traits<buffer_type>::entry_type entry_type;
                    entry_type& entry = *static_cast<entry_type*>(prev);
					return *static_cast<T*>(backend::buffer_traits<buffer_type>::template get_object<sizeof(T), std::alignment_of<T>::value>(entry));
                    
					return *static_cast<value_type*>(prev);
				}        

				// only fails if version is invalid. Throws exception in that case
				slot_offset_t current_active = acquire_for_read(txb.version()); 
				// register us in buffer immediately, so that if an error occurs from now on, the object gets released when it is popped from buffer
				txb.open_r(*this); 

				// get the value from the slot we got access to
				const value_type& result = static_cast<const derived&>(*this)[current_active];
				return result;
			}

			value_type& active_slot(){
				detail::scoped_access_version guard;
				return static_cast<derived&>(*this)[active_slot_id()];
			}
		};
	}
}

#endif
