#include <stm.hpp>
#include <stm/orelse.hpp>

#include <catch.hpp>

namespace {
  struct retry_func {
    retry_func(bool& has_run) : has_run(has_run) {}
    void operator()(stm::transaction& tx) {
      bool tmp = has_run;
      has_run = true;
      if (!tmp) { tx.retry(); }
    }

    bool& has_run;
  };

  struct nested_retry {
    nested_retry(bool& has_run) : has_run(has_run) {}
    void operator()(stm::transaction&) {
      stm::atomic(retry_func(has_run));
    }

    bool& has_run;

  };

  struct success_func {
    success_func(bool& has_run) : has_run(has_run) {}
    void operator()(stm::transaction&) {
      has_run = true;
    }

    bool& has_run;
  };

}

TEST_CASE("orelse", "orelse tests")
{
	SECTION("basic", "not sure, it seems like this should block, since we orelse between two tx'es who both retry")
	{ // todo: maybe retry doesn't currently block, but just polls? That would explain it, but then we're really testing for wrong behavior
		bool first_ran = false;
		bool second_ran = false;
		stm::atomic(stm::orelse(retry_func(first_ran), retry_func(second_ran))); 
		REQUIRE(first_ran);
		REQUIRE(second_ran);
	}

	SECTION("nested-retry-orelse-retry", "as above, not sure")
	{
		bool first_ran = false;
		bool second_ran = false;
		stm::atomic(stm::orelse(nested_retry(first_ran), retry_func(second_ran)));
		REQUIRE(first_ran);	
		REQUIRE(second_ran);
	}

	SECTION("retry-orelse-nested-retry", "as above, not sure")
	{
		bool first_ran = false;
		bool second_ran = false;
		stm::atomic(stm::orelse(retry_func(second_ran), nested_retry(first_ran)));
		REQUIRE(first_ran);	
		REQUIRE(second_ran);
	}

	SECTION("(retry-orelse-retry)-orelse-nested-retry", "again, not sure")
	{
		bool first_ran = false;
		bool second_ran = false;
		bool third_ran = false;
		stm::atomic(stm::orelse(stm::orelse(retry_func(first_ran), retry_func(second_ran)), nested_retry(third_ran)));
		REQUIRE(first_ran);	
		REQUIRE(second_ran);
		REQUIRE(third_ran);
	}

	SECTION("three-way-orelse", "'or'ing 3 retries together under a single orelse")
	{
		bool first_ran = false;
		bool second_ran = false;
		bool third_ran = false;
		stm::atomic(stm::orelse(retry_func(first_ran), retry_func(second_ran)) || nested_retry(third_ran));
		REQUIRE(first_ran);	
		REQUIRE(second_ran);
		REQUIRE(third_ran);
	}

	SECTION("success-orelse-retry", "if first succeeds, orelse := first")
	{
		bool first_ran = false;
		bool second_ran = false;
		bool third_ran = false;
		stm::atomic(stm::orelse(success_func(first_ran), retry_func(second_ran)) || retry_func(third_ran));
		REQUIRE(first_ran);	
		REQUIRE(!second_ran);
		REQUIRE(!third_ran);
	}

	SECTION("retry-orelse-success", "if second succeeds, orelse := second")
	{
		bool first_ran = false;
		bool second_ran = false;
		bool third_ran = false;
		stm::atomic(stm::orelse(retry_func(first_ran), success_func(second_ran)) || retry_func(third_ran));
		REQUIRE(first_ran);	
		REQUIRE(second_ran);
		REQUIRE(!third_ran);
	}
}
