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

	void test();
	
	void setUp();
	void tearDown();

	static CppUnit::Test* suite();

private:
};


#endif // WORKERTEST_H_INCLUDED
