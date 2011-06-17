#ifndef NNODETEST_H_INCLUDED
#define NNODETEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class NNodeTest: public CppUnit::TestCase
{
public:
    NNodeTest(const std::string & name);

    ~NNodeTest();

    void testNode();

    void setUp();

    void tearDown();

    static CppUnit::Test* suite();
};


#endif // NNODETEST_H_INCLUDED
