#ifndef NODECALCULATORTEST_H_INCLUDED
#define NODECALCULATORTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Futile/TypedWrapper.h"
#include "Futile/WorkerPool.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class NodeCalculatorTest: public CppUnit::TestCase
{
public:
	NodeCalculatorTest(const std::string & name);

	~NodeCalculatorTest();

    void testNodeCalculator();

    Futile_TypedWrapper(Depth, int);
    Futile_TypedWrapper(Width, int);
    Futile_TypedWrapper(WorkerCount, int);
    Futile_TypedWrapper(TimeMs, int);

    void testInterrupt(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs);

    void testDestroy(Futile::Worker & inMainWorker, Futile::WorkerPool & inWorkerPool);
	
	void setUp();

	void tearDown();

	static CppUnit::Test* suite();
};


#endif // NODECALCULATORTEST_H_INCLUDED
