#ifndef STM_TRANSACTION_MANAGER_HPP
#define STM_TRANSACTION_MANAGER_HPP

#include "backend.hpp"
#include "buffer_functions.hpp"
#include "utility.hpp"
#include "transaction_group.hpp"

#include <numeric> 
#include <vector>

namespace stm {
	namespace frontend {
		struct snapshot_val {
			snapshot_val() {}
			snapshot_val(void (*assign)(const void* src, void* dest), const void* src, void* dest) : assign(assign), src(src), dest(dest) {}

			void (*assign)(const void* src, void* dest);
			const void* src;
			backend::shared_base* top_dest;
			void* dest;
		};

		// every thread has an associated transaction manager, which contains the tx-local buffer and other transaction infrastructure
		template <typename buffer_type>
		struct transaction_manager { 
			// iterators for traversing the buffer's read-only and read-write elements respectively
			typedef typename buffer_type::ptr_position offset_r;
			//typedef typename buffer_type::generic_position offset_rw;
			typedef std::vector<std::pair<backend::shared_base*, void*>> lut_container;
			typedef typename backend::buffer_traits<buffer_type>::entry_type buffer_entry_type;
			//transaction_manager(const buffer_type buf&) : snapshots(), buf(buf), orelse_count(0) {}
			transaction_manager(buffer_type&& buf, tx_group<transaction_manager<buffer_type>>& grp) : snapshots(), buf(std::move(buf)), orelse_count(0), grp(&grp) {}
			transaction_manager(tx_group<transaction_manager<buffer_type>>& grp) : snapshots(), buf(), orelse_count(0), grp(&grp) {}

			transaction_manager() : snapshots(), buf(), orelse_count(0), grp(NULL) {}

			transaction_manager(transaction_manager&& other) : buf(), orelse_count(0), grp(other.grp) {
				std::swap(snapshots, other.snapshots);
				std::swap(buf, other.buf);
				std::swap(orelse_count, other.orelse_count);
				std::swap(buffer_lookup, other.buffer_lookup);
				std::swap(lut_delims, other.lut_delims);
			}
			transaction_manager& operator=(transaction_manager&& other) {
				grp = other.grp;
				std::swap(snapshots, other.snapshots);
				std::swap(buf, other.buf);
				std::swap(orelse_count, other.orelse_count);
				std::swap(buffer_lookup, other.buffer_lookup);
				std::swap(lut_delims, other.lut_delims);
			}

			// returns true if none of the objects between the top of the read-buffer and the supplied iterator have been updated after the tx started
			bool validate(offset_r pos_r, version_field_t tx_version) throw() {
				// custom algorithm: validate everything added since our tx started
				buffer_type& buf = buffer();
				bool result = buffer().all(pos_r, [tx_version, &buf](void* ptr) {
					return !ver_ops::shared_valid_in_tx(static_cast<backend::shared_base*>(ptr)->version(), tx_version); 
				});	
				return result;
			}

			// release locks on opened read-only objects
			void release_read_lock(offset_r first, version_field_t tx_version) throw() {
				buffer().for_each_ptr(first, [tx_version](void* ptr){
					auto& shd = *static_cast<backend::shared_base*>(ptr);
					slot_offset_t locked_slot = shd.reader_registered_slot(tx_version);
					shd.release_reader(locked_slot);
				});
			}
			// todo: why are these two members of manager at all?
			// release locks taken during commit on objects opened for writing 
			template <typename iter_t> // traverses lut table, not buffer
			void release_commit_lock_on_rollback(iter_t first, iter_t last) throw() {
				std::for_each(first, last, [](std::pair<backend::shared_base*, void*> src){src.first->release_unchanged();});
			}
			template <typename iter_t> // traverses lut table, not buffer
			void release_commit_lock_on_commit(iter_t first, iter_t last, version_field_t commit_version) throw() {
				std::for_each(first, last, [commit_version](std::pair<backend::shared_base*, void*> src){ src.first->release_updated(commit_version); });
			}

			// calls the destructor on objects stored in the buffer
			void destroy(lut_container::iterator first) throw() {
				{ 
					detail::scoped_access_version guard;
				}

				std::for_each(first, buffer_lookup.end(), [](std::pair<backend::shared_base*, void*> item){
					backend::buffer_traits<buffer_type>::destroy(static_cast<buffer_entry_type*>(item.second));
				});
			}

			// Opens an object for writing. Handles if the object is already open safely
			template <typename T> // todo: might want to factor out outer_open further, so we only do branch at highest possible level
			typename backend::buffer_traits<buffer_type>::entry_type* open_rw(backend::shared_base& handle, const T& obj, destroy_type destroy, assign_type assign) {
				typedef backend::buffer_traits<buffer_type> traits;
				auto* entry = buffer().template allocate_generic<sizeof(T), std::alignment_of<T>::value>();
				traits::initialize(*entry, obj, destroy, assign, &handle, NULL);
				//T* ptr = static_cast<T*>(traits::get_object<sizeof(T), std::alignment_of<T>::value>(*entry));
				return entry;
			}

			template <typename T>
			typename backend::buffer_traits<buffer_type>::entry_type* open_rw_from_parent(backend::shared_base& handle, void* outer_open, destroy_type destroy, assign_type assign) {
				typedef backend::buffer_traits<buffer_type> traits;
				T* src = static_cast<T*>(traits::template get_object<sizeof(T), std::alignment_of<T>::value>(*static_cast<typename traits::entry_type*>(outer_open)));
				
				auto* entry = buffer().template allocate_generic<sizeof(T), std::alignment_of<T>::value>();
				traits::initialize(*entry, *src, destroy, assign, &handle, outer_open);
				//T* obj = static_cast<T*>(traits::get_object<sizeof(T), std::alignment_of<T>::value>(*entry));
				return entry;
			}

			// Register  an object in buffer as opened for reading
			void open_r(const backend::shared_base& handle) throw() {
				void* dest = buffer().allocate_ptr();
				*static_cast<const backend::shared_base**>(dest) = &handle;
			}

			// return end or beginning of this table
			lut_container::iterator lut_begin(std::vector<size_t>::reverse_iterator lut_index) {
				return buffer_lookup.begin() + *(lut_index);
			}
			lut_container::iterator lut_end(std::vector<size_t>::reverse_iterator lut_index) {
				return lut_index == lut_delims.rbegin() ? buffer_lookup.end() : lut_begin(--lut_index);
			}
			// Searches a series of lookup tables defiend by [first, last). If the object is found, then a pointer to it is returned
			// otherwsie return null
			void* find(std::vector<size_t>::iterator first, std::vector<size_t>::iterator last, const backend::shared_base* obj){
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
						, [](std::pair<const backend::shared_base* const, const void* const> pair, const backend::shared_base* obj)
					{ return std::less<const backend::shared_base* const>()(pair.first, obj); });
					if (res != lut_last && res->first == obj) { return res->second; }
				}

				return NULL;
			}

			// All non-nested transactions update this field, so nested transactions can retrieve it
			version_field_t last_version;

			// indicates that the transaction currently strating is an orelse
			void orelse_starting() throw(){ ++orelse_count; }
			// indicates that an orelse is ending
			void orelse_ending() throw(){ --orelse_count; }

			bool is_orelse() const throw() { return orelse_count > 0; }

			// number of nested transactions currently
			size_t depth() { return lut_delims.size(); }

			bool is_empty() {
				return buffer().empty();
			}

			buffer_type& buffer() { return buf; }
			tx_group<transaction_manager<buffer_type>>& group() { return *grp; }

			std::vector<snapshot_val> snapshots; // kind of hackish to make it a public var, but it'll do for now
			lut_container buffer_lookup;
			std::vector<size_t> lut_delims; // stores the start iter of each lut 

		private:
			transaction_manager(const transaction_manager&);
			transaction_manager& operator=(const transaction_manager&);
			buffer_type buf;
			int orelse_count;
			tx_group<transaction_manager<buffer_type>>* grp;
		};

		typedef transaction_manager<config::default_buffer> default_manager_type;

		inline default_manager_type& get_manager() throw();

		inline tx_group<default_manager_type>& default_tx_group() throw() {
			static tx_group<default_manager_type> default_group;
			return default_group;
		}
	}
}

#endif
