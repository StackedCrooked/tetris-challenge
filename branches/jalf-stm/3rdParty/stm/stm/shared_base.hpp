#ifndef STM_SHARED_BASE_HPP
#define STM_SHARED_BASE_HPP

#include "version_ops.hpp"
#include "header.hpp"

#include "config.hpp"
#include "utility.hpp"

#include <algorithm>

namespace stm {
	namespace backend {
		struct shared_base {
			shared_base() throw();

			shared_base(const shared_base& other);

		private:
			shared_base& operator=(const shared_base& other);
		public:

			// attempt to lock the object, preventing updates until lock is released again
			// if the operation fails, the object is left unchanged
			bool lock_for_commit(version_field_t tx_version) throw();

			// undo lock_for_commit
			void release_updated(version_field_t commit_version) throw();
			void release_unchanged() throw();
			

			// if shared has been updated so the local version is higher than the one of the transaction,
			// this throws an exception (and reverts any changes),
			// otherwise it succeeds, and returns the currently active slot id
			slot_offset_t acquire_for_read(version_field_t tx_version) const;

			// undo acquire_for_read
			void release_reader(slot_offset_t slot_id) const throw();

			slot_offset_t reader_registered_slot(version_field_t tx_version) throw();

			void update_version_and_flip(version_field_t ver) throw();

			version_field_t version() throw() { return ver_ops::version_from_shared(header()); }

			slot_offset_t active_slot_id() throw() { return ver_ops::active_offset(header()); }

			int16_t lock_val(slot_offset_t slot){ return lock[slot]; }

			version_field_t header_val() const { return header(); }

		private:
			detail::header_t header;
			mutable int16_t lock[2];
		};
	}
}
#endif
