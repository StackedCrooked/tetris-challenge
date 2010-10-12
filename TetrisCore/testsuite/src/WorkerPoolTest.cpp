#include "WorkerPoolTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/WorkerPool.h"
#include "Poco/Thread.h"
#include <boost/thread.hpp>
#include <iostream>


using namespace Tetris;


WorkerPoolTest::WorkerPoolTest(const std::string & inName):
    CppUnit::TestCase(inName),
    mRepeat(100)
{
}


WorkerPoolTest::~WorkerPoolTest()
{
}


void WorkerPoolTest::printProgress(size_t a, size_t b)
{
    std::cout << (a + 1) << "/" << b << "\r";
}


void WorkerPoolTest::CountTo(Poco::UInt64 inNumber)
{
    for (Poco::UInt64 idx = 0; idx != inNumber; ++idx)
    {
        boost::this_thread::interruption_point();
        Poco::Thread::sleep(200);
    }
}


void WorkerPoolTest::testSimple()
{
    std::cout << "1" << std::endl;
    WorkerPool pool("Johnny", 1);
    assert(pool.getWorker() == pool.getWorker());

    std::cout << "2" << std::endl;
    pool.setSize(2);
    assert(pool.getWorker() != pool.getWorker());

    std::cout << "3" << std::endl;
    pool.setSize(1);
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.interruptAndClearQueue();

    std::cout << "4" << std::endl;
    pool.setSize(2);
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.interruptAndClearQueue();

    std::cout << "5" << std::endl;
    pool.setSize(3);
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.interruptAndClearQueue();

    std::cout << "6" << std::endl;
    pool.setSize(3);
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.interruptAndClearQueue();

    std::cout << "7" << std::endl;
    pool.setSize(3);
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    pool.interruptAndClearQueue();

    std::cout << "8" << std::endl;
    pool.setSize(10);
    for (size_t idx = 0; idx != 10; ++idx)
    {
        pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    }
    Poco::Thread::sleep(10);
    pool.interruptAndClearQueue();

    std::cout << "9" << std::endl;
    pool.setSize(10);
    for (size_t idx = 0; idx != 20; ++idx)
    {
        pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    }
    Poco::Thread::sleep(10);
    pool.interruptAndClearQueue();

    std::cout << "10" << std::endl;
    pool.setSize(200);
    for (size_t idx = 0; idx != 300; ++idx)
    {
        pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    }    
    Poco::Thread::sleep(10);
    pool.interruptAndClearQueue();

    std::cout << "11" << std::endl;
    pool.setSize(200);
    for (size_t idx = 0; idx != 500; ++idx)
    {
        pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    }    
    Poco::Thread::sleep(10);
    pool.setSize(0);

    std::cout << "12" << std::endl;
    std::cout << "Destructor is taking too long..." << std::endl;
    pool.setSize(50);
    for (size_t idx = 0; idx != 50; ++idx)
    {
        pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::CountTo, 1000 * 1000));
    }    
    Poco::Thread::sleep(10);
}


void WorkerPoolTest::setUp()
{
}


void WorkerPoolTest::tearDown()
{
}


CppUnit::Test * WorkerPoolTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("WorkerPoolTest"));
	CppUnit_addTest(suite, WorkerPoolTest, testSimple);
	return suite;
}
