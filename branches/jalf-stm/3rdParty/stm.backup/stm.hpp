#ifndef STM_HPP
#define STM_HPP

#include "stm/config.hpp"

STM_BEGIN_SILENCE_MMX_WARNING

#include "stm/transaction_manager.hpp"
#include "stm/transaction.hpp"
#include "stm/shared.hpp"
#include "stm/shared_detached.hpp"
#include "stm/atomic.hpp"
#include "stm/orelse.hpp"
#include "stm/utility.hpp"

#ifdef STM_HEADER_ONLY
// draw in the implementation of transaction
#include "stm/transaction_impl.hpp"
#endif

STM_END_SILENCE_MMX_WARNING

#endif
