#ifndef GENERICGRID2TEST_H_INCLUDED
#define GENERICGRID2TEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class GenericGrid2Test: public CppUnit::TestCase
{
public:
	GenericGrid2Test(const std::string & name);

	~GenericGrid2Test();

    void test();

    void testAll();
	
	void setUp();

	void tearDown();

	static CppUnit::Test* suite();
};


#endif // GENERICGRID2TEST_H_INCLUDED
