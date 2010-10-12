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


void WorkerTest::testStatus()
{
    Tetris::Worker worker("TestWorker");
    assertEqual(worker.status(), Tetris::Worker::Status_Nil);

    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);

    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);

    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 0);
}


void WorkerTest::testSimpleInterrupt()
{
    Tetris::Worker worker("TestWorker");
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 0);
    
    worker.interruptOne();
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);
}


void WorkerTest::testAdvancedInterrupt()
{
    Tetris::Worker worker("TestWorker");
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 1);

    worker.interruptOne();
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 0);

    worker.interruptOne();
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);

    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    Poco::Thread::sleep(10);
    worker.interruptAll();
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);
}


void WorkerTest::setUp()
{
}


void WorkerTest::tearDown()
{
}


CppUnit::Test * WorkerTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("WorkerTest"));
	CppUnit_addTest(suite, WorkerTest, testStatus);
	CppUnit_addTest(suite, WorkerTest, testSimpleInterrupt);
	CppUnit_addTest(suite, WorkerTest, testAdvancedInterrupt);
	return suite;
}
