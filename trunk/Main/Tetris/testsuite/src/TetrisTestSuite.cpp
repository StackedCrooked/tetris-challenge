#include "TetrisTestSuite.h"
#include "WorkerTest.h"
#include "WorkerPoolTest.h"
#include "NodeCalculatorTest.h"
#include "GenericGridTest.h"
#include "GenericNodeTest.h"


CppUnit::Test* TetrisCoreTestSuite::suite()
{
    CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("TetrisCoreTestSuite");

    pSuite->addTest(GenericNodeTest::suite());
    pSuite->addTest(GenericGridTest::suite());
    pSuite->addTest(WorkerTest::suite());
    pSuite->addTest(WorkerPoolTest::suite());
    pSuite->addTest(NodeCalculatorTest::suite());
    return pSuite;
}
