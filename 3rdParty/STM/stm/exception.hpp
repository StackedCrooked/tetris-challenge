#ifndef STM_EXCEPTION_HPP
#define STM_EXCEPTION_HPP

namespace stm {
	// Used whenever a conflict is detected forcing a transaction to roll back
	class conflict_exception 
	{};

  // a validation conflict occurred while opening an object
  class conflict_on_open : public conflict_exception 
	{};

  // a conflict occurred during commit. Either an object could not be locked, or its version was invalid
  class conflict_on_commit : public conflict_exception 
	{
  public:
    enum error_type { lock_failed, validate_failed };
    conflict_on_commit(error_type error) : error(error) {}
    error_type error; 
  };

  // user has called retry()
	class retry_exception : public conflict_exception
	{};

	// user has called abort()
	class abort_exception 
	{};
}
#endif
