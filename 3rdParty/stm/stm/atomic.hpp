#ifndef STM_ATOMIC_HPP
#define STM_ATOMIC_HPP

#include "transaction.hpp"
#include "tls.hpp"

#include <cstdio>

namespace stm {
	namespace detail {
		// helper class so that main atomic loop can be reused for void and non-void return types
		// these specializations encapsulate the lines of code that need to change
		template <typename R, typename F>
		struct atomic_helper {
			static R commit_and_return(transaction& tx, F& f){
				R result = f(tx); 
				frontend::transaction_internal& txb = detail::inner_tx(tx);
				frontend::commit(txb);
				return result;
			}
		};

		template <typename F>
		struct atomic_helper<void, F> {
			static void commit_and_return(transaction& tx, F& f){
				f(tx);
				frontend::transaction_internal& txb = detail::inner_tx(tx);
				frontend::commit(txb);
			}
		};

		// main atomic function
		template <typename R, typename F>
		R atomic(F f, bool is_orelse = false){
			auto& mgr = frontend::get_manager();
			// set up orelse, if relevant
			
			if (is_orelse) {
				mgr.orelse_starting();
			}
			auto end_orelse = scope_guard([&mgr, is_orelse]() {
				if (is_orelse) {
					mgr.orelse_ending();
				}
			});
			for (;;) {
				bool is_outer = mgr.depth() == 0;
				auto tx_ver = is_outer ? mgr.group().get_current_version() : mgr.last_version;
				if (is_outer) { // todo: branch should be unnecessary here
					mgr.last_version = tx_ver;
				}

				frontend::transaction_internal txb(mgr, tx_ver, is_orelse);
				transaction tx(txb);
				try {
					return atomic_helper<R, F>::commit_and_return(tx, f);
				} 
				catch (const retry_exception&){
					if (frontend::in_orelse(txb)){ // just ask mgr directly?
						throw;
					}
					else {
						// do the same as conflict ex 
						if (!frontend::is_outer(txb)){ // just ask mgr directly?
							throw;
						}
					}
				}
				catch (const conflict_exception&){
					if (!frontend::is_outer(txb)){
						throw;
					}
				} 
				catch (const abort_exception&){
					throw;
				}
				// do nothing on other exceptions. When tx goes out of scope, it is automatically aborted if commit has not been called
				catch(...){ // deal with user-thrown exceptions
					try {
						// if a user exception is thrown, try to commit
						frontend::commit(txb);
						throw; // and then rethrow
					}
					catch (const conflict_exception&) {
						if (!frontend::is_outer(txb)){
							throw;
						}
					}
				}
			}
		}
	}

	// lambdas not yet implemented in GCC, and in MSVC they don't handle scoping correctly, 
	// making it impossible to deduce return type with decltype in the general case.
	// so commented out for now
#ifdef LAMBDAS_PROPERLY_IMPLEMENTED
#ifdef STM_USE_CPP0X
	template <typename F>
	auto atomic(F f) -> decltype(f(detail::make_instance<stm::transaction&>())){
		return detail::atomic<decltype(f(detail::make_instance<stm::transaction&>())), F>(f);
	}
#else
	template <typename F>
	void atomic(F f){
		detail::atomic<void, F>(f);
	}
#endif
	template <typename R, typename F>
	R atomic(F f){
		return detail::atomic<R, F>(f);
	}
#else
	// for now, just use the old C++03 implementation, where return type has to be explicitly specified
	template <typename F>
	void atomic(F f){
		detail::atomic<void, F>(f);
	}

	template <typename R, typename F>
	R atomic(F f){
		return detail::atomic<R, F>(f);
	}
#endif
}

#endif
