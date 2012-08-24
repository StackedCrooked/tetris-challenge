#ifndef H_BACKEND_73D532579D5B3E4CBE495D8BBF8238D1
#define H_BACKEND_73D532579D5B3E4CBE495D8BBF8238D1

#include "config.hpp"

STM_BEGIN_SILENCE_MMX_WARNING

//#include "iterator_rw.hpp"
#include "shared_base.hpp"
#include "list_buffer.hpp"
#include "array_buffer.hpp"
#include "fixed_buffer.hpp"
#include "version_ops.hpp"
#include "header.hpp"

namespace stm {
	namespace backend {
		struct shared_base;
		class list_buffer;
	}
}

#if defined(STM_HEADER_ONLY)
#include "backend_impl.hpp"
#endif

STM_END_SILENCE_MMX_WARNING

#endif
