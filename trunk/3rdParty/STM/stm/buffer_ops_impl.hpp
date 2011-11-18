#ifndef H_BUFFER_OPS_IMPL_A49E12790A27144DAF6F4F29425FB575
#define H_BUFFER_OPS_IMPL_A49E12790A27144DAF6F4F29425FB575

#include "buffer_ops.hpp"

namespace stm {
	namespace backend {
		STM_LIB_FUNC void call_destroy::operator()(shared_base*& src) throw() {
			metadata* meta = reinterpret_cast<metadata*>(&src);
			meta->destroy(meta);
		}
		STM_LIB_FUNC void call_rollback_release::operator()(std::pair<shared_base*, metadata*> src) throw() {
			src.first->release_unchanged();
		}
		STM_LIB_FUNC call_commit_release::call_commit_release(version_field_t commit_version) throw() : commit_version(commit_version) {}
		STM_LIB_FUNC void call_commit_release::operator()(std::pair<shared_base*, metadata*> src) throw() {
			src.first->release_updated(commit_version);
		}
		STM_LIB_FUNC call_release_reader::call_release_reader(version_field_t tx_version) throw() : tx_version(tx_version) {}
		STM_LIB_FUNC void call_release_reader::operator()(shared_base*& src) throw() {
			slot_offset_t locked_slot = src->reader_registered_slot(tx_version);
			src->release_reader(locked_slot);
		}
	}
}


#endif
