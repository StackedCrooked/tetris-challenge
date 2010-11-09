#ifndef GENERICNODETEST_H_INCLUDED
#define GENERICNODETEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class GenericGridTest: public CppUnit::TestCase
{
public:
	GenericGridTest(const std::string & name);

	~GenericGridTest();

    void testGenericGrid();    
	
	void setUp();

	void tearDown();

	static CppUnit::Test* suite();
};


#endif // GENERICNODETEST_H_INCLUDED
