#include <stm/utility.hpp>
#include <stm/shared_base.hpp> 
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Utility)

BOOST_AUTO_TEST_CASE( ptr_cast )
{
	int i = 0;
	float* p = stm::ptr_cast<float*>(&i);
	BOOST_CHECK_EQUAL(*p, 0.0f);
}

BOOST_AUTO_TEST_CASE( ptr_cast_roundtrip )
{
	int i = 42;
	long double* p = stm::ptr_cast<long double*>(&i);
	BOOST_CHECK_EQUAL(*stm::ptr_cast<int*>(p), 42);
}

BOOST_AUTO_TEST_CASE( ptr_cast_const )
{
	const int i = 0;
	const float* p = stm::ptr_cast<const float*>(&i);
	BOOST_CHECK_EQUAL(*p, 0.0f);
}

BOOST_AUTO_TEST_CASE( ptr_cast_roundtrip_const )
{
	const int i = 42;
	const long double* p = stm::ptr_cast<const long double*>(&i);
	BOOST_CHECK_EQUAL(*stm::ptr_cast<const int*>(p), 42);
}

BOOST_AUTO_TEST_SUITE_END()
