#ifndef GENERICGRIDTEST_H_INCLUDED
#define GENERICGRIDTEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class GenericGridTest: public CppUnit::TestCase
{
public:
    GenericGridTest(const std::string & name);

    ~GenericGridTest();

    void testGenericGrid();

    void setUp();

    void tearDown();

    static CppUnit::Test* suite();
};


#endif // GENERICGRIDTEST_H_INCLUDED
