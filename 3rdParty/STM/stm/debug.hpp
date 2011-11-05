#ifndef H_DEBUG_ADD49102D0BEAA47A595CCBE428510FA
#define H_DEBUG_ADD49102D0BEAA47A595CCBE428510FA

#include <stm.hpp>

namespace stm {
	namespace debug {
		inline bool is_buffer_empty() { return stm::frontend::get_manager().is_empty(); }
		inline stm::uint64_t current_version()  {
			detail::scoped_access_version guard;
			return stm::detail::basic_ver_ops<stm::version_field_t>::from_version(stm::frontend::default_tx_group().get_current_version());
		}
	}
}

#endif
