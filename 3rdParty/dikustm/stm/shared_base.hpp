#ifndef STM_SHARED_BASE_HPP
#define STM_SHARED_BASE_HPP

#include "shared_ops.hpp"

#include "platform_specific.hpp"
#include "exception.hpp"
#include "config.hpp"
#include "utility.hpp"

#include <algorithm>

namespace stm {
	namespace backend {
		inline void version_fail_on_open(){
			throw conflict_on_open(); // might be able to handle by refreshing tx version
		}

		struct shared_base {
			shared_base() throw() : header() {
				lock[0] = 0;
				lock[1] = 0;
			}

			shared_base(const shared_base& other) : header(other.header) {
				std::copy(other.lock, other.lock + 2, lock);
			}

		private:
			shared_base& operator=(const shared_base& other);
		public:

			// attempt to lock the object, preventing updates until lock is released again
			// if the operation fails, the object is left unchanged
			template <typename notifier_t>
			bool lock_for_commit(notifier_t& ntf) throw() { 
				using namespace detail;
				int result = 0;
				// register on both slots - we could register both, check to see which slot is active, and then unregister that, but would be a lot of overhead for no reason
				for (int i = 0; i < config::commit_acquire_retries; ++i){
					int16_t first = atomic_ops::increment(lock[0]);
					int16_t second = atomic_ops::increment(lock[1]);

					version_field_t hdr = header();

					// get number of readers on the inactive slot (if it is 1, we've got exclusive ownership of it and can proceed)
					result = (active_offset(hdr) == 0) ? second : first;
					if (result != 1){
						// either retry immediately or if that doesn't work, fire cond var and then go to sleep
						release(ntf);
						backoff(i);
						continue;
					}
					break;
				}
				// if the result is 1 it means we incremented from 0, so the backing slot was free, and we successfully locked
				return result == 1;
			}

			// undo lock_for_commit
			template <typename notifier_t>
			void release(notifier_t& ntf) throw() {
				int16_t i = atomic_ops::decrement(lock[0]);
				int16_t j = atomic_ops::decrement(lock[1]);

				if (i == 0 || j == 0){
					ntf(*this); //todo:condvar: ideally, this should only wake up if inactive slot == 0 (or when tx_ver < shared_ver)
				}
			}

			// if shared has been updated so the local version is higher than the one of the transaction,
			// this throws an exception (and reverts any changes),
			// otherwise it succeeds, and returns the currently active slot id
			template <typename notifier_t>
			slot_offset_t acquire_for_read(version_field_t tx_version, notifier_t& ntf){
				version_field_t hdr = header();
				version_field_t ver = detail::get_version(hdr);

				if (ver > tx_version){ 
					version_fail_on_open();
				}

				slot_offset_t active_id = detail::active_offset(hdr);
				atomic_ops::increment(lock[active_id]);

				if (detail::get_version(header()) > tx_version){ // handle has been updated in the meantime
					// undo our changes
					slot_offset_t id = this->active_slot_id();
					if (atomic_ops::decrement(lock[active_id]) == 0 && id != active_id) {
						ntf(*this);
					}
					version_fail_on_open();
				}

				return active_id;

			}

			// undo acquire_for_read
			template <typename notifier_t>
			void release_reader(slot_offset_t slot_id, notifier_t& ntf) throw() {
				slot_offset_t id = this->active_slot_id();
				if (atomic_ops::decrement(lock[slot_id]) == 0 && id != slot_id) {
					ntf(*this);
				}
			}
			slot_offset_t get_locked_slot(version_field_t tx_version) throw() {
				version_field_t hdr = header();
				// if the object has not been updated since we registered
				slot_offset_t slot;
				if (detail::get_version(hdr) <= tx_version){
					// the slot we registered on is still active
					slot = detail::active_offset(hdr);
				}
				else {
					// the slot we registered on is no longer the active one
					slot = detail::inactive_offset(hdr);
				}
				return slot;
			}

			void update_version_and_flip(version_field_t ver) throw() {
				// we assume that the new version has 0 in the msb used to indicate active slot, so we don't need to mask it out
				version_field_t hdr = header();
				// mask out all version bits, and swap slot bit of the old version
				version_field_t new_slot_mask = ~hdr & (static_cast<version_field_t>(1) << ((sizeof(version_field_t) * 8)-1));
				header(ver | new_slot_mask);
				assert(detail::active_offset(hdr) != detail::active_offset(header()));
			}

			version_field_t version() throw() {
				return detail::get_version(header());
			}

			slot_offset_t active_slot_id() throw() {
				return detail::active_offset(header());
			}

			int16_t lock_val(slot_offset_t slot){ return lock[slot]; }

			version_field_t header_val() const { return header(); }

		private:
			detail::header_t header;
			int16_t lock[2];
		};
	}
}
#endif
