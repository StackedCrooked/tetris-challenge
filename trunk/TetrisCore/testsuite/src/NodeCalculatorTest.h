#ifndef NODECALCULATORTEST_H_INCLUDED
#define NODECALCULATORTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class NodeCalculatorTest: public CppUnit::TestCase
{
public:
	NodeCalculatorTest(const std::string & name);

	~NodeCalculatorTest();

    void testNodeCalculator();    
	
	void setUp();

	void tearDown();

	static CppUnit::Test* suite();
};


#endif // NODECALCULATORTEST_H_INCLUDED
