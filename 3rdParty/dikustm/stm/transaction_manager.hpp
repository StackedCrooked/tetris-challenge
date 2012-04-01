#ifndef STM_TRANSACTION_MANAGER_HPP
#define STM_TRANSACTION_MANAGER_HPP

#include "shared_base.hpp"
#include "buffer_entry.hpp"
#include "buffer_functions.hpp"
#include "utility.hpp"
#include "transaction_manager_ops.hpp"
#include "buffer_ops.hpp"

#include <boost/thread/tss.hpp>
#include <numeric> 

namespace stm {
	namespace frontend {
		// every thread has an associated transaction manager, which contains the tx-local buffer and other transaction infrastructure
		template <typename buffer_t>
		struct transaction_manager { 
      // iterators for traversing the buffer's read-only and read-write elements respectively
			typedef typename buffer_t::iterator_r iterator_r;
			typedef typename buffer_t::iterator_rw iterator_rw;

			typedef std::pair<typename buffer_t::iterator_r, typename buffer_t::iterator_rw> tx_marker;

			transaction_manager(buffer_t buffer = buffer_t()) : buffer(buffer), orelse_count(0), tx_depth(0) {}

			// returns a marker containing current read and copy iterators. This is used to denote beginning of a new transaction
			tx_marker begin() throw() { 
				return std::make_pair(buffer.begin_r(), buffer.begin_rw());
			}

			// returns a marker pointing to the beginning of the outermost transaction
			tx_marker end() throw() { 
				return std::make_pair(buffer.end_r(), buffer.end_rw());
			}

			// returns true if none of the objects between the top of the buffer and the supplied marker have been updated after the tx started
			bool validate(tx_marker marker, version_field_t tx_version) throw() {
				bool read_result = std::accumulate(buffer.begin_r(), marker.first, true, validator(tx_version));
				bool result = read_result && std::accumulate(buffer.begin_rw(), marker.second, true, validator(tx_version));
				return result;
			}

      // release locks on opened read-only objects
			template <typename notifier_t>
			void release(iterator_r last, version_field_t tx_version, notifier_t& ntf) throw() {
				buffer.release(last, tx_version, ntf);
			}
      // release locks taken during commit on objects opened for writing 
			template <typename iter_t, typename notifier_t>
			void release(iter_t first, iter_t last, notifier_t& ntf) throw() {
				std::for_each(first, last, backend::call_release<notifier_t>(ntf));
			}

      // calls the destructor on objects stored in the buffer
			void destroy(iterator_rw last) throw() {
				buffer.destroy(last);
			}

      // remove objects from the buffer up to the specified marker
			void pop(tx_marker marker){
				buffer.pop(marker.first);
				buffer.pop(marker.second);
			}
      // remove objects from the buffer up to the specified marker
			void pop(iterator_rw end){
				buffer.pop(end);
			}

      // number of objects in the transaction's writeset
			size_t writeset_size(iterator_rw last) { return buffer.write_count(last); }

			// Not called from anywhere currently, but could be exposed to the user
      // Assumes that the object specified is not already opened by this or a parent transaction
			template <typename T>
			T& unique_open_rw(backend::shared_base& handle, T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
				T* result = buffer.push_rw(&handle, obj, destroy, assign);
				assert(result != NULL);
				return *result;
			}

      // Opens an object for writing. Handles if the object is already open safely
			template <typename T>
			T& open_rw(backend::shared_base& handle, backend::metadata* outer_open, T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
				T* val = &obj;
				if (outer_open != NULL) {
					val = backend::get_object<T>(outer_open);
				}
				T* result = buffer.push_rw(&handle, outer_open, *val, destroy, assign);
				assert(result != NULL);
				return *result;
			}

			// Register  an object in buffer as opened for reading
			void open_r(backend::shared_base& handle) throw() {
				buffer.push_r(&handle);
			}

      // search in the buffer for a specified object
			backend::metadata* find(iterator_rw first, iterator_rw last, backend::shared_base* obj){
				iterator_rw res = std::find(first, last, obj);
				return res == last ? NULL : ptr_cast<backend::metadata*>(&*res);
			}

			// All non-nested transactions update this field, so nested transactions can retrieve it
			version_field_t last_version;

      // indicates to the manager that a new transaction is starting, and returns true if the starting transaction is non-nested
			bool is_new_tx_outer() throw() { return ++tx_depth == 1; }

      // indicates that the transaction currently strating is an orelse
			void orelse_starting() throw(){ ++orelse_count; }
      // indicates that an orelse is ending
			void orelse_ending() throw(){ --orelse_count; }

			bool is_orelse() throw() { return orelse_count > 0; }
			
      // number of nested transactions currently
      int& depth() { return tx_depth; }

		private:
			buffer_t buffer;
			int orelse_count;
			int tx_depth;
		};
	}
}

#endif
