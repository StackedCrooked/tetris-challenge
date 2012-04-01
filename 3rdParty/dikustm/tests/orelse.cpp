#include <stm.hpp>
#include <stm/orelse.hpp>

#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_SUITE( orelse_test )

BOOST_AUTO_TEST_CASE ( simple_orelse)
{
  bool first_ran = false;
  bool second_ran = false;
  stm::atomic(stm::orelse(retry_func(first_ran), retry_func(second_ran)));
  BOOST_CHECK(first_ran);
  BOOST_CHECK(second_ran);

}

BOOST_AUTO_TEST_CASE ( nested_first)
{
  bool first_ran = false;
  bool second_ran = false;
  stm::atomic(stm::orelse(nested_retry(first_ran), retry_func(second_ran)));
  BOOST_CHECK(first_ran);	
  BOOST_CHECK(second_ran);
}

BOOST_AUTO_TEST_CASE ( nested_second)
{
  bool first_ran = false;
  bool second_ran = false;
  stm::atomic(stm::orelse(retry_func(second_ran), nested_retry(first_ran)));
  BOOST_CHECK(first_ran);	
  BOOST_CHECK(second_ran);
}

BOOST_AUTO_TEST_CASE ( nested_orelse)
{
  bool first_ran = false;
  bool second_ran = false;
  bool third_ran = false;
  stm::atomic(stm::orelse(stm::orelse(retry_func(first_ran), retry_func(second_ran)), nested_retry(third_ran)));
  BOOST_CHECK(first_ran);	
  BOOST_CHECK(second_ran);
  BOOST_CHECK(third_ran);
}

BOOST_AUTO_TEST_CASE ( three_way_orelse)
{
  bool first_ran = false;
  bool second_ran = false;
  bool third_ran = false;
  stm::atomic(stm::orelse(retry_func(first_ran), retry_func(second_ran)) || nested_retry(third_ran));
  BOOST_CHECK(first_ran);	
  BOOST_CHECK(second_ran);
  BOOST_CHECK(third_ran);
}

BOOST_AUTO_TEST_CASE ( first_ok)
{
  bool first_ran = false;
  bool second_ran = false;
  bool third_ran = false;
  stm::atomic(stm::orelse(success_func(first_ran), retry_func(second_ran)) || retry_func(third_ran));
  BOOST_CHECK(first_ran);	
  BOOST_CHECK(!second_ran);
  BOOST_CHECK(!third_ran);
}

BOOST_AUTO_TEST_CASE ( second_ok)
{
  bool first_ran = false;
  bool second_ran = false;
  bool third_ran = false;
  stm::atomic(stm::orelse(retry_func(first_ran), success_func(second_ran)) || retry_func(third_ran));
  BOOST_CHECK(first_ran);	
  BOOST_CHECK(second_ran);
  BOOST_CHECK(!third_ran);
}


BOOST_AUTO_TEST_SUITE_END()
