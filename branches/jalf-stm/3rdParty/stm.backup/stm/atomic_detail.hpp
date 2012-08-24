#ifndef STM_ATOMIC_DETAIL_HPP
#define STM_ATOMIC_DETAIL_HPP

#include "transaction.hpp"

#include <cstdio>
#include <map>
#include <cmath>

namespace stm {
  template <typename T>
  T fake();

  struct atomic_detail {
    atomic_detail() : commits(), open_failures(), commit_lock_failures(), commit_validate_failures(), user_rollbacks() {}

#ifdef FIGURED_OUT_DECLTYPE
    template <typename F>
    auto operator()(F f) -> decltype(f(detail::make_instance<stm::transaction&>())){
      return this->atomic<decltype(f(detail::make_instance<stm::transaction&>())), F>(f);
    }
#endif

    template <typename F>
    void operator()(F f){
      this->atomic<void, F>(f);
    }
    template <typename R, typename F>
    R operator()(F f){
      return this->atomic<R, F>(f);
    }

    typedef unsigned int count_t;
    count_t commits;
    count_t open_failures;
    count_t commit_lock_failures;
    count_t commit_validate_failures;
    count_t user_rollbacks;
  private:

    template <typename R, typename F>
    struct atomic_detail_helper {
      static R commit_and_return(transaction& tx, F& f, count_t& commits){
        R result = f(tx); 
        frontend::transaction_internal& txb = detail::inner_tx(tx);
        txb.commit();
        ++commits;
        return result;
      }
    };

    template <typename F>
    struct atomic_detail_helper<void, F> {
      static void commit_and_return(transaction& tx, F& f, count_t& commits){
        f(tx);
        frontend::transaction_internal& txb = detail::inner_tx(tx);
        txb.commit();
        ++commits;
      }
    };

    template <typename R, typename F>
    R atomic(F f, bool is_orelse = false){
	  auto& mgr = frontend::get_manager();			
	  for (;;) {
		bool is_outer = mgr.depth() == 0;
		auto tx_ver = is_outer ? mgr.group().get_current_version() : mgr.last_version;
		if (is_outer) { // todo: branch should be unnecessary here
		  mgr.last_version = tx_ver;
		}
		if (is_orelse){
		  mgr.orelse_starting();
		}

		frontend::transaction_internal txb(mgr, tx_ver, is_orelse);
		transaction tx(txb);
		try {
          return atomic_detail_helper<R, F>::commit_and_return(tx, f, commits);
        }
        catch (const retry_exception&){
          if (txb.is_in_orelse()){
            throw;
          }
          else {
            // do the same as conflict ex 
            ++user_rollbacks;
            if (!txb.is_outer()){
              throw;
            }
          }
        }
        catch (const conflict_on_open&){
          ++open_failures;
          if (!txb.is_outer()){
            throw;
          }
        }
        catch (const conflict_on_commit& ex){
          if (ex.error == conflict_on_commit::lock_failed) {
            ++commit_lock_failures;
          }
          else {
            ++commit_validate_failures;
          }
          if (!txb.is_outer()){
            throw;
          }
        }
        catch (const abort_exception&){
          ++user_rollbacks;
          throw;
        }
        // do nothing on other exceptions. When tx goes out of scope, it is automatically aborted if commit has not been called
        catch(...){ // deal with user-thrown exceptions
          try { // yuck, there's got to be a cleaner way to do this
            txb.commit();
            ++commits;
            throw;
          }
          catch (const conflict_on_open&){
            ++open_failures;
            if (!txb.is_outer()){
              throw;
            }
          }
          catch (const conflict_on_commit& ex){
            if (ex.error == conflict_on_commit::lock_failed) {
              ++commit_lock_failures;
            }
            else {
              ++commit_validate_failures;
            }

            if (!txb.is_outer()){
              throw;
            }
          }
        }
      }
    }
  };
}

#endif
