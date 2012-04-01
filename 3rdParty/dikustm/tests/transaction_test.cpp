#include <stm.hpp>
#include <boost/test/unit_test.hpp>

namespace {
	stm::shared<int> val(0);
}

BOOST_AUTO_TEST_SUITE( Transaction )

BOOST_AUTO_TEST_CASE( commit_rw_test )
{
	stm::transaction tx;
	stm::frontend::shared_internal<int>& valb = stm::detail::open_shared(val);
	int old_val = valb.active_slot();

	++val.open_rw(tx);
  stm::frontend::transaction_internal& txb = stm::detail::inner_tx(tx);        
	txb.commit();

	BOOST_CHECK_EQUAL(valb.lock_val(0), 0);
	BOOST_CHECK_EQUAL(valb.lock_val(1), 0);
	BOOST_CHECK_EQUAL(valb.version(), stm::frontend::default_tx_group().get_current_version());
	BOOST_CHECK_EQUAL(valb.active_slot(), old_val+1);
}

BOOST_AUTO_TEST_CASE( rollback_rw_test )
{
	stm::transaction tx;
	stm::frontend::shared_internal<int> valb = stm::detail::open_shared(val);
	stm::version_field_t ver = valb.version();
	int old_val = valb.active_slot();

	++val.open_rw(tx);
	BOOST_CHECK_THROW(tx.abort(), stm::abort_exception);

	BOOST_CHECK_EQUAL(valb.lock_val(0), 0);
	BOOST_CHECK_EQUAL(valb.lock_val(1), 0);
	BOOST_CHECK_EQUAL(valb.version(), ver);
	BOOST_CHECK_EQUAL(valb.active_slot(), old_val);
}

BOOST_AUTO_TEST_CASE( commit_r_test )
{
	stm::transaction tx;
	stm::frontend::shared_internal<int>& valb = stm::detail::open_shared(val);
	stm::version_field_t ver = valb.version();
	int old_val = valb.active_slot();

	val.open_r(tx);
  stm::frontend::transaction_internal& txb = stm::detail::inner_tx(tx);
	txb.commit();

	BOOST_CHECK_EQUAL(valb.lock_val(0), 0);
	BOOST_CHECK_EQUAL(valb.lock_val(1), 0);
	BOOST_CHECK_EQUAL(valb.version(), ver);
	BOOST_CHECK_EQUAL(valb.active_slot(), old_val);
}

BOOST_AUTO_TEST_CASE( rollback_r_test )
{
	stm::transaction tx;
	stm::frontend::shared_internal<int> valb = stm::detail::open_shared(val);
	stm::version_field_t ver = valb.version();
	int old_val = valb.active_slot();

	val.open_r(tx);
	BOOST_CHECK_THROW(tx.abort(), stm::abort_exception);

	BOOST_CHECK_EQUAL(valb.lock_val(0), 0);
	BOOST_CHECK_EQUAL(valb.lock_val(1), 0);
	BOOST_CHECK_EQUAL(valb.version(), ver);
	BOOST_CHECK_EQUAL(valb.active_slot(), old_val);
}

BOOST_AUTO_TEST_SUITE_END()
