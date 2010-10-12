#ifndef WORKERPOOLTEST_H_INCLUDED
#define WORKERPOOLTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Types.h"


class WorkerPoolTest: public CppUnit::TestCase
{
public:
	WorkerPoolTest(const std::string & name);
	~WorkerPoolTest();

    static void CountTo(Poco::UInt64 inNumber);

	void testSimple();

    void setUp();

    void tearDown();

    static CppUnit::Test* suite();

private:
    void printProgress(size_t a, size_t b);
    size_t mRepeat;
};


#endif // WORKERPOOLTEST_H_INCLUDED
