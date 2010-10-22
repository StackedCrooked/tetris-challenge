#include "TetrisCoreTestSuite.h"
#include "WorkerTest.h"
#include "WorkerPoolTest.h"
#include "NodeCalculatorTest.h"


CppUnit::Test* TetrisCoreTestSuite::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("TetrisCoreTestSuite");

	pSuite->addTest(NodeCalculatorTest::suite());
	pSuite->addTest(WorkerTest::suite());
	pSuite->addTest(WorkerPoolTest::suite());

	return pSuite;
}
