#ifndef NODECALCULATORTEST_H_INCLUDED
#define NODECALCULATORTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Tetris/TypedWrapper.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class NodeCalculatorTest: public CppUnit::TestCase
{
public:
	NodeCalculatorTest(const std::string & name);

	~NodeCalculatorTest();

    void testNodeCalculator();

    Tetris_TypedWrapper(Depth, int);
    Tetris_TypedWrapper(Width, int);
    Tetris_TypedWrapper(WorkerCount, int);
    Tetris_TypedWrapper(TimeMs, int);

    void test(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs);
	
	void setUp();

	void tearDown();

	static CppUnit::Test* suite();
};


#endif // NODECALCULATORTEST_H_INCLUDED
