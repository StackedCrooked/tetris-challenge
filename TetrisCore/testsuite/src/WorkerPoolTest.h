#ifndef WORKERPOOLTEST_H_INCLUDED
#define WORKERPOOLTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class WorkerPoolTest: public CppUnit::TestCase
{
public:
	WorkerPoolTest(const std::string & name);
	~WorkerPoolTest();

    static void BeBusy();

	void testWorkerPool();

    void setUp();

    void tearDown();

    static CppUnit::Test* suite();

private:
    Poco::Stopwatch mStopwatch;
};


#endif // WORKERPOOLTEST_H_INCLUDED
