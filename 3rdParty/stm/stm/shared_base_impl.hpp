#ifndef H_SHARED_BASE_IMPL_3A57705F63A7FE42BE4FE63182424047
#define H_SHARED_BASE_IMPL_3A57705F63A7FE42BE4FE63182424047

#include "config.hpp"
#include "shared_base.hpp"
#include "exception.hpp"

namespace stm {
	namespace backend {
		STM_LIB_FUNC void version_fail_on_open(){
			throw conflict_on_open(); // might be able to handle by refreshing tx version
		}

		STM_LIB_FUNC shared_base::shared_base() throw() : header() {
			detail::scoped_access_version guard;
			lock[0] = 0;
			lock[1] = 0;
		}

		STM_LIB_FUNC shared_base::shared_base(const shared_base& other) : header(other.header) {
			detail::scoped_access_version guard;
			std::copy(other.lock, other.lock + 2, lock);
		}

		// attempt to lock the object, preventing updates until lock is released again
		// if the operation fails, the object is left unchanged
		STM_LIB_FUNC bool shared_base::lock_for_commit(version_field_t tx_version) throw() {  // why no version check? Is it because we do it later? Why not do it now anyway?
			version_field_t hdr = header();
			// this version must be the right one. If another stealth update happens, we won't be able to succeed.
			// should verify this. Put some kind of trace on the alt case, see if we ever manage to commit once we enter it.
			if (!ver_ops::shared_valid_in_tx(hdr, tx_version)) {
				return false;
			}

			slot_offset_t locked_id = ver_ops::inactive_offset(hdr);

			// try a couple of times, because another thread might just temporarily lock in order to safely read version field, so reasonable chance it'll be free again in a few cycles
			for (size_t i = 0; i < config::commit_acquire_retries; ++i) {	
				auto count = atomic_ops::cas(lock[locked_id], 1, 0);
				if (count != 0) {
					continue;
				}
				if (ver_ops::hdr_neq(hdr, header())) {
					// decrement
					atomic_ops::decrement(lock[locked_id]);
					break;
				}
				return true;
			}
			return false;
		}

		// undo lock_for_commit
		STM_LIB_FUNC void shared_base::release_updated(version_field_t commit_version) throw() {
			version_field_t hdr = header(); // this version id might have been updated again. We set it to commit_version, is it > that? (key is that our versionflip changed active slot and so our comit lock became a read lock
			slot_offset_t slot = (ver_ops::ver_neq(hdr, commit_version)) ? ver_ops::inactive_offset(hdr) : ver_ops::active_offset(hdr);
			atomic_ops::decrement(lock[slot]);
		}
		STM_LIB_FUNC void shared_base::release_unchanged() throw() {
			// so we rolled back. But we held the lock until now, and we haven't updated version, so our lock is still a commit lock, meaning version must be unchanged
			version_field_t hdr = header(); 
			slot_offset_t slot = ver_ops::inactive_offset(hdr);
			atomic_ops::decrement(lock[slot]);
		}

		// if shared has been updated so the local version is higher than the one of the transaction,
		// this throws an exception (and reverts any changes),
		// otherwise it succeeds, and returns the currently active slot id
		STM_LIB_FUNC slot_offset_t shared_base::acquire_for_read(version_field_t tx_version) const { // todo: lock_for_commit really needs the same but for other slot. Can we unify? (note a few subtle active/inactive and !=/== diffs
			version_field_t hdr = header();

			if (!ver_ops::shared_valid_in_tx(hdr, tx_version)){ 
				version_fail_on_open();
			}
			// "guess" at currently active slot. Based on nonatomic read, so might be wrong, but if it is, we need to rollback
			slot_offset_t active_id = ver_ops::active_offset(hdr);
			atomic_ops::increment(lock[active_id]);	

			if (ver_ops::hdr_neq(hdr, header())){ // handle has been updated in the meantime
				// undo our changes
				atomic_ops::decrement(lock[active_id]);
				version_fail_on_open();
			}

			return active_id;
		}

		// undo acquire_for_read
		STM_LIB_FUNC void shared_base::release_reader(slot_offset_t slot_id) const throw() {
			atomic_ops::decrement(lock[slot_id]);
		}

		STM_LIB_FUNC slot_offset_t shared_base::reader_registered_slot(version_field_t tx_version) throw() {
			version_field_t hdr = header();
			return ver_ops::reader_registered_on(hdr, tx_version);
		}

		STM_LIB_FUNC void shared_base::update_version_and_flip(version_field_t ver) throw() {
			version_field_t hdr = header();
			version_field_t new_hdr = ver_ops::set_version_and_flip(hdr, ver);
			header(new_hdr); 
		}

		STM_LIB_FUNC void update_and_flip(shared_base& shd, version_field_t commit_version) {
			shd.update_version_and_flip(commit_version);
		}

	}
}

#endif
