#ifndef MEMORYPOOLTEST_H_INCLUDED
#define MEMORYPOOLTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class MemoryPoolTest: public CppUnit::TestCase
{
public:
    MemoryPoolTest(const std::string & name);

    ~MemoryPoolTest();

    void testMemoryPool();

    void testMemoryPoolPerformance();

    void setUp();

    void tearDown();

    static CppUnit::Test* suite();

private:
    static void BeBusy();
    Poco::Stopwatch mStopwatch;
    size_t mRepeat;
};


#endif // MEMORYPOOLTEST_H_INCLUDED
