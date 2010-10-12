#include "TetrisCoreTestSuite.h"
#include "WorkerTest.h"
#include "WorkerPoolTest.h"


CppUnit::Test* TetrisCoreTestSuite::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("TetrisCoreTestSuite");

	pSuite->addTest(WorkerTest::suite());
	//pSuite->addTest(WorkerPoolTest::suite());

	return pSuite;
}
