#ifndef WORKERTEST_H_INCLUDED
#define WORKERTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Types.h"


class WorkerTest: public CppUnit::TestCase
{
public:
	WorkerTest(const std::string & name);
	~WorkerTest();

    static void CountTo(Poco::UInt64 inNumber);

	void testStatus();
	void testStatusImpl();

	void testSimpleInterrupt();
	void testSimpleInterruptImpl();

	void testAdvancedInterrupt();
	void testAdvancedInterruptImpl();
    
	
	void setUp();
	void tearDown();

	static CppUnit::Test* suite();

private:
    void printProgress(size_t a, size_t b);
    size_t mRepeat;
};


#endif // WORKERTEST_H_INCLUDED
