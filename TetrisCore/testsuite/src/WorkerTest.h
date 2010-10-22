#ifndef WORKERTEST_H_INCLUDED
#define WORKERTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class WorkerTest: public CppUnit::TestCase
{
public:
	WorkerTest(const std::string & name);

	~WorkerTest();

    void testWorker();    

    void testWorkerImpl();
	
	void setUp();

	void tearDown();

	static CppUnit::Test* suite();

private:
    static void BeBusy();
    void printProgress(size_t a, size_t b);
    Poco::Stopwatch mStopwatch;
    size_t mRepeat;
};


#endif // WORKERTEST_H_INCLUDED
