#ifndef STM_TRANSACTION_MANAGER_HPP
#define STM_TRANSACTION_MANAGER_HPP

#include "backend.hpp"
#include "buffer_functions.hpp"
#include "utility.hpp"
#include "transaction_manager_ops.hpp"

#include <boost/thread/tss.hpp>
#include <numeric> 
#include <vector>

namespace stm {
	namespace frontend {
		struct snapshot_val {
			snapshot_val() {}
			typedef void (*assign_func)(const void*, void*);
			snapshot_val(assign_func f, const void* src, void* dest) : f(f), src(src), dest(dest) {}

			assign_func f;
			const void* src;
			void* dest;
		};


		// every thread has an associated transaction manager, which contains the tx-local buffer and other transaction infrastructure
		template <typename buffer_t>
		struct transaction_manager { 
			// iterators for traversing the buffer's read-only and read-write elements respectively
			typedef typename buffer_t::iterator_r iterator_r;
			typedef typename buffer_t::iterator_rw iterator_rw;
			typedef std::vector<std::pair<backend::shared_base*, backend::metadata*>> lut_container;


			typedef std::pair<typename buffer_t::iterator_r, typename buffer_t::iterator_rw> tx_marker;

			transaction_manager(buffer_t buffer = buffer_t()) : snapshots(), buffer(buffer), orelse_count(0), tx_depth(0) {}

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
				bool result = read_result;
				return result;
			}

			// release locks on opened read-only objects
			void release_read_lock(iterator_r last, version_field_t tx_version) throw() {
				buffer.release(last, tx_version);
			}
			// release locks taken during commit on objects opened for writing 
			template <typename iter_t>
			void release_commit_lock_on_rollback(iter_t first, iter_t last) throw() {
				std::for_each(first, last, backend::call_rollback_release());
			}
			template <typename iter_t>
			void release_commit_lock_on_commit(iter_t first, iter_t last, version_field_t commit_version) throw() {
				std::for_each(first, last, backend::call_commit_release(commit_version));
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
			template <typename T> // todo: might want to factor out outer_open further, so we only do branch at highest possible level
			backend::metadata* open_rw(backend::shared_base& handle, const T& obj, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
				const T* val = &obj;
				backend::metadata* result = push_rw(buffer, &handle, NULL, *val, destroy, assign);
				assert(result != NULL);
				return result;
			}

			template <typename T>
			backend::metadata* open_rw_from_parent(backend::shared_base& handle, backend::metadata* outer_open, void (*destroy)(backend::metadata*), void (*assign)(const backend::metadata* psrc, void* pdst)) {
				const T* val = backend::get_object<T>(outer_open);
				backend::metadata* result = push_rw(buffer, &handle, outer_open, *val, destroy, assign);
				assert(result != NULL);
				return result;
			}


			// Register  an object in buffer as opened for reading
			void open_r(backend::shared_base& handle) throw() {
				buffer.push_r(&handle);
			}

			// return end or beginning of this table
			lut_container::iterator lut_begin(std::vector<size_t>::reverse_iterator lut_index) {
				//return lut_index == lut_delims.end() ? buffer_lookup.end() : buffer_lookup.begin() + *(lut_index);
				return buffer_lookup.begin() + *(lut_index);
				//return lut_index == lut_delims.end() ? buffer_lookup.end() : lut_begin(--lut_index);
			}
			lut_container::iterator lut_end(std::vector<size_t>::reverse_iterator lut_index) {
				// return end or the beginning of next table
				return lut_index == lut_delims.rbegin() ? buffer_lookup.end() : lut_begin(--lut_index);
				//return lut_index == lut_delims.end() ? buffer_lookup.end() : buffer_lookup.begin() + *(lut_index);
			}
			// Searches a series of lookup tables defiend by [first, last). If the object is found, then a pointer to it is returned
			// otherwsie return null
			backend::metadata* find(std::vector<size_t>::iterator first, std::vector<size_t>::iterator last, backend::shared_base* obj){
				// so the lut index iterators we want to traverse:
				auto rfirst = std::vector<size_t>::reverse_iterator(last);
				auto rlast = std::vector<size_t>::reverse_iterator(first);

				// now, for each lut in this range, get lut begin/end iterators
				// and then search this range, using a binary search with pred

				for (auto cur = rfirst; cur != rlast; ++cur) {
					auto lut_first = lut_begin(cur);
					auto lut_last = lut_end(cur);
					// if found in this lut, return it
					auto res = std::lower_bound(lut_first
						, lut_last
						, obj
						, [](std::pair<const backend::shared_base* const, const backend::metadata* const> pair, const backend::shared_base* obj)
					{ return std::less<const backend::shared_base* const>()(pair.first, obj); });
					if (res != lut_last && res->first == obj) { return res->second; }
				}

				return NULL;
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

			bool is_empty() {
				return buffer.begin_r() == buffer.end_r() && buffer.begin_rw() == buffer.end_rw();
			}

			std::vector<snapshot_val> snapshots; // kind of hackish to make it a public var, but it'll do for now
			lut_container buffer_lookup;
			std::vector<std::size_t> lut_delims; // stores the start iter of each lut 

		private:
			buffer_t buffer;
			int orelse_count;
			int tx_depth;
		};

		typedef transaction_manager<backend::fixed_array_buffer > default_manager_type;

	}
}

#endif
