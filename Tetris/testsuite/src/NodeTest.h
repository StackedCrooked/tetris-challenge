#ifndef NODETEST_H_INCLUDED
#define NODETEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class NodeTest: public CppUnit::TestCase
{
public:
    NodeTest(const std::string & name);

    ~NodeTest();

    void testNode();

    void setUp();

    void tearDown();

    static CppUnit::Test* suite();
};


#endif // NODETEST_H_INCLUDED
