#include "MemoryPoolTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/MemoryPool.h"
#include "Futile/Threading.h"
#include "Futile/Assert.h"
#include "Poco/Thread.h"
#include "Poco/Stopwatch.h"
#include <iostream>



using namespace Futile::Memory::Pool;


static const int cSleepTimeMs = 100;


MemoryPoolTest::MemoryPoolTest(const std::string & inName):
    CppUnit::TestCase(inName),
    mStopwatch(),
    mRepeat(10)
{
}


MemoryPoolTest::~MemoryPoolTest()
{
}


void MemoryPoolTest::setUp()
{
}


void MemoryPoolTest::tearDown()
{
}


namespace { // anonymous namespace


struct Point
{

    Point() : x(0), y(0) {}

    Point(int inX, int inY) : x(inX), y(inY) {}

    int x;
    int y;
};

static Point * CreatePoint(void * placement)
{
    return new (placement) Point(1, 2);
}

struct Buffer
{
    char onebyte[256];

    typedef SharedPtr<Buffer> Ptr;
};


} // anonymous namespace


void MemoryPoolTest::testMemoryPoolPerformance()
{
//    typedef MemoryPool<Buffer> MemoryPool;

//    const Poco::UInt64 cKiloByte = 1024;
//    const Poco::UInt64 cMegaByte = 1024 * cKiloByte;
//    const Poco::UInt64 cGigaByte = 1024 * cMegaByte;

//    {
//        MemoryPool pool(100 * 1000);
//        std::vector<Buffer::Ptr> thePointers;
//        thePointers.reserve(pool.size());
//        std::cout << "Vector size before: " << thePointers.size() << std::endl;

//        Futile::Stopwatch theStopwatch;
//        for (Poco::UInt64 idx = 0; idx < pool.size(); ++idx)
//        {
//            thePointers.push_back(Share(AcquireAndDefaultConstruct(pool)));
//        }
//        int elapsedMs = theStopwatch.elapsedTimeMs();

//        std::cout << "Vector size after: " << thePointers.size() << std::endl;

//        std::cout << "You are currently high on memory usage. Check it out.";
//        std::cin.get();


//        std::cout << "The pool should occupy " << (sizeof(Buffer) *  pool.used() / cMegaByte) << " MB\n";
//        std::cin.get();
//    }
//    std::cout << "The pool has been dropped.";
//    std::cin.get();

}


void MemoryPoolTest::testMemoryPool()
{
    std::cout << std::endl;

    const std::size_t cItemCount = 100;
    const std::size_t cByteCount = cItemCount * sizeof(Point);

    typedef MemoryPool<Point> MemoryPool;

    MemoryPool pool(cItemCount);

    assertEqual(pool.size(), cByteCount);

    assertEqual(pool.used(), std::size_t(0));

    assertEqual(pool.available(), cItemCount);

    Point * point = new (pool.acquire()) Point(3, 4);

    assertEqual(pool.indexOf(point), std::size_t(0));

    assertEqual(point->x, 3);
    assertEqual(point->y, 4);

    point->~Point();


    pool.release(point);

    assertEqual(pool.size(), cByteCount);
    assertEqual(pool.used(), std::size_t(0));
    assertEqual(pool.available(), cItemCount);


    {
        MemoryPool pool(cItemCount);
        assertEqual(pool.used(), 0);
        assertEqual(pool.available(), cItemCount);
        {
            MovePtr<Point> ptr1(AcquireAndDefaultConstruct(pool));
            assertEqual(pool.used(), 1);
            assertEqual(pool.available(), cItemCount - 1);

            MovePtr<Point> ptr2(AcquireAndDefaultConstruct(pool));
            assertEqual(pool.used(), 2);
            assertEqual(pool.available(), cItemCount - 2);
        }
        assertEqual(pool.used(), 0);
        assertEqual(pool.available(), cItemCount);
    }

    {

        MemoryPool pool(cItemCount);
        {
            assertEqual(pool.used(), 0);

            SharedPtr<Point> outer_0(pool);
            assertEqual(pool.used(), 0);

            assertEqual(outer_0.get(), NULL);

            SharedPtr<Point> outer_1(Share(AcquireAndDefaultConstruct(pool)));
            assertEqual(pool.used(), 1);
            Assert(outer_1.get());
            Assert(outer_0.get() != outer_1.get());

            {
                SharedPtr<Point> inner_1(Share(AcquireAndDefaultConstruct(pool)));

                assertEqual(pool.used(), 2);

                inner_1.reset(outer_1.get());

                assertEqual(pool.used(), 1);

                outer_1 = inner_1;

                assertEqual(pool.used(), 1);
            }
        }
        assertEqual(pool.used(), 0);

        assertEqual(pool.available(), cItemCount);
    }

    std::vector<Point*> points;
    for (std::size_t idx = 0; idx < cItemCount; ++idx)
    {

        MovePtr<Point> point(AcquireAndConstructWithFactory(pool, CreatePoint));
        point->x = 1;
        point->y = 2;

        assertEqual(pool.size(), cByteCount);
        assertEqual(pool.used(), idx + 1);
        assertEqual(pool.available(), cItemCount - idx - 1);
        assertEqual(pool.indexOf(point), idx);
        assertEqual(pool.offsetOf(point), sizeof(Point) * idx);

        Assert(point.get());
        points.push_back(point.release());
    }

    while (pool.used() > 0)
    {
        assertEqual(pool.used(), points.size());
        DestructAndRelease(pool, points.back());
        points.pop_back();
    }
}


CppUnit::Test * MemoryPoolTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("MemoryPoolTest"));
    CppUnit_addTest(suite, MemoryPoolTest, testMemoryPool);
    CppUnit_addTest(suite, MemoryPoolTest, testMemoryPoolPerformance);
    return suite;
}
