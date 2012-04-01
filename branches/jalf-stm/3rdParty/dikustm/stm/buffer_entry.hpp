#ifndef STM_BUFFER_ENTRY_HPP
#define STM_BUFFER_ENTRY_HPP

#include "platform_specific.hpp"
#include "config.hpp"

namespace stm {
	namespace backend {
		struct shared_base;
		// represents all the metadata associated with a transaction-local copy, but not the object copy itself
		// - how to copy and destroy it, and which shared handle it is tied to
		struct metadata { 
			metadata(shared_base* src, metadata* reopen_tail, metadata* tail, void (*destroy)(metadata*), void (*assign)(const metadata*, void*)) throw() 
				: src(src)
				, reopen_tail(reopen_tail)
				, destroy(destroy)
				, assign(assign)
				, tail(tail) {}

			shared_base* src;
			metadata* reopen_tail;
			void (*destroy)(metadata*); 
			void (*assign)(const metadata*, void*);

			metadata* tail; // pointer to the previous object allocated in the buffer
		};

		// a complete transaction-local copy, containing metadata as well as sufficient (and properly aligned) POD storage for the object itself
		template <size_t size, size_t align>
		struct buffer_entry {

			template <typename T>
			buffer_entry(shared_base* src, metadata* prev_open, const T& obj, metadata* tail, void (*destroy)(metadata*), void (*assign)(const metadata*, void*)) throw() 
				: meta(src, prev_open, tail, destroy, assign)
			{
				new (&storage) T(obj);
			}
			typedef typename detail::aligned_storage<size, align>::type storage_type;
			metadata meta; // metadata associated with the object
			storage_type storage; // storage for the copy of the object
		};

    // convenience function for getting the object stored along with a given metadata pointer
		template <typename T>
		T* get_object(metadata* meta) {
			typedef buffer_entry<sizeof(T), detail::alignment_of<T>::value> local_t;
			local_t* local = ptr_cast<local_t*>(meta);
			return ptr_cast<T*>(&(local->storage));
		}
	}
}

#endif
