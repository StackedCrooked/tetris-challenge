#ifndef GENERICNODETEST_H_INCLUDED
#define GENERICNODETEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class GenericNodeTest: public CppUnit::TestCase
{
public:
    GenericNodeTest(const std::string & name);

    ~GenericNodeTest();

    void testNode();

    void setUp();

    void tearDown();

    static CppUnit::Test* suite();
};


#endif // GENERICNODETEST_H_INCLUDED
