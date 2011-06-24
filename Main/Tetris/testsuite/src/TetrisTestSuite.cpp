#include "TetrisTestSuite.h"
#include "MemoryPoolTest.h"
#include "WorkerTest.h"
#include "WorkerPoolTest.h"
#include "NodeCalculatorTest.h"
#include "NodeTest.h"
#include "GenericGridTest.h"


CppUnit::Test* TetrisCoreTestSuite::suite()
{
    CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("TetrisCoreTestSuite");

    pSuite->addTest(NodeTest::suite());
    pSuite->addTest(MemoryPoolTest::suite());
    pSuite->addTest(GenericGridTest::suite());
    pSuite->addTest(WorkerTest::suite());
    pSuite->addTest(WorkerPoolTest::suite());
    pSuite->addTest(NodeCalculatorTest::suite());
    return pSuite;
}
