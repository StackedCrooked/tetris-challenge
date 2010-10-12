#include "WorkerTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/Worker.h"
#include "Poco/Thread.h"
#include <boost/thread.hpp>
#include <iostream>


WorkerTest::WorkerTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


WorkerTest::~WorkerTest()
{
    std::cout << "~WorkerTest()" << std::endl;
}


void WorkerTest::CountTo(Poco::UInt64 inNumber)
{
    for (Poco::UInt64 idx = 0; idx != inNumber; ++idx)
    {
        boost::this_thread::interruption_point();
        Poco::Thread::sleep(10);
    }
}

void Print(const std::string & inMessage)
{
    std::cout << inMessage << "\n";
}

void WorkerTest::test()
{
    Tetris::Worker worker("TestWorker");

    Print("1");
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000*1000));
    Poco::Thread::sleep(100);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 0);

    Print("2");
    worker.interruptOne();
    Poco::Thread::sleep(100);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);

    Print("3");    
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000*1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000*1000));
    Poco::Thread::sleep(100);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 1);

    Print("4");
    worker.interruptOne();
    Print("4.1");
    Poco::Thread::sleep(100);
    Print("4.2");
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    Print("4.3");
    assertEqual(worker.size(), 0);
    Print("4.4");

    Print("5");
    worker.interruptOne();
    Poco::Thread::sleep(100);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);

    std::cout << "END of test()";
}


void WorkerTest::setUp()
{
    Print("setUp");
}


void WorkerTest::tearDown()
{
    Print("tearDown");
}


CppUnit::Test* WorkerTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("WorkerTest");
	CppUnit_addTest(pSuite, WorkerTest, test);
	return pSuite;
}
