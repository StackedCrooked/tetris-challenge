#ifndef STM_TRANSACTION_INTERNAL_OPS_HPP
#define STM_TRANSACTION_INTERNAL_OPS_HPP

#include "shared_base.hpp"

namespace stm {
	namespace frontend {
    // attempts to lock all objects opened for writing, in preparation for committing
		template <typename iterator_rw, typename notifier_t>
		iterator_rw lock(iterator_rw first, iterator_rw last, notifier_t& ntf){
			iterator_rw it = first;
			while (it != last && (**it)->lock_for_commit(ntf)){
				++it;
			}
			if (it != last){
				iterator_rw nit = first;
				while (nit != it){
					(**nit)->release(ntf);
					++nit;
				}
				return it;
			}
			return last;
		}

    // applies speculative changes to the canonical objects and sets the object's version to the specified value
		struct apply {
			apply(version_field_t version) throw() : version(version) {}
			void operator()(backend::shared_base** base) { 
				backend::metadata* meta = ptr_cast<backend::metadata*>(base);
				meta->assign(meta, meta->src);
				meta->src->update_version_and_flip(version);
			}

			version_field_t version;
		};

    // applies speculative changes to a copy placed in the buffer by an outer transaction
		struct nested_apply {
			nested_apply(backend::metadata*& first, backend::metadata*& prev) throw() : first(first), prev(prev) {}
			void operator()(backend::shared_base*& base) {
				backend::metadata* meta = ptr_cast<backend::metadata*>(&base);
				// if the object has not been opened before, do nothing about it
				if (meta->reopen_tail == NULL) {
					if (first == NULL) {
						first = meta;
					}
					prev = meta;
					return;
				}
				// else, apply changes to the "parent" copy, destroy this object, and adjust the previous object visited
        // so it points to the object after this one, effectively removing this from the list
				prev->tail = meta->tail;
				meta->assign(meta, meta->reopen_tail);
				meta->destroy(meta);
			}

			backend::metadata*& first;
			backend::metadata*& prev;
		};

    // Helper class: Calls release when it goes out of scope
		template <typename manager_type, typename iter_type, typename notifier_t>
		struct release_writer_guard : boost::noncopyable {
			typedef typename manager_type::tx_marker marker_type;
			release_writer_guard(manager_type& manager, iter_type first, iter_type last, notifier_t& ntf) : manager(manager), first(first), last(last), ntf(ntf) {}
			~release_writer_guard(){
				// release write list
				manager.release(first, last, ntf);
			}

			manager_type& manager;
			iter_type first;
			iter_type last;
			notifier_t& ntf;
		};

    // helper class: Destroys/releases objects and removes them from the buffer when it goes out of scope
		template <typename manager_type, typename notifier_t>
		struct pop_guard : boost::noncopyable {
			typedef typename manager_type::tx_marker marker_type;
			pop_guard(manager_type& manager, marker_type marker, version_field_t version, bool& is_live, notifier_t& ntf) : manager(manager), marker(marker), version(version), is_live(is_live), ntf(ntf) {}
			~pop_guard(){
				// release readers
				manager.release(marker.first, version, ntf);
				// destroy writers
				manager.destroy(marker.second);
				// pop readers and writers
				manager.pop(marker);
				is_live = false;
			}

			manager_type& manager;
			marker_type marker;
			version_field_t version;
			bool& is_live;
			notifier_t& ntf;
		};

		template <typename T>
		void direct_assign(const void* src, void* dest) {
			const T& s = *reinterpret_cast<const T*>(src);
			T& d = *reinterpret_cast<T*>(dest);
			d = s;
		}
	}
}

#endif
