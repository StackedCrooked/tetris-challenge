#ifndef H_BUFFER_ENTRY_IMPL_20CA2EDAAEAA4B41BA21D1056635A401
#define H_BUFFER_ENTRY_IMPL_20CA2EDAAEAA4B41BA21D1056635A401

#include "buffer_entry.hpp"

namespace stm {
	namespace backend {
		STM_LIB_FUNC metadata::metadata(shared_base* src, metadata* reopen_tail, metadata* tail, void (*destroy)(metadata*), void (*assign)(const metadata*, void*)) throw() 
			: src(src)
			, reopen_tail(reopen_tail)
			, destroy(destroy)
			, assign(assign)
			, tail(tail) {}
	}
}


#endif
