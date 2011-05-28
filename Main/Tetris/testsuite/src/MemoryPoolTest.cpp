#include "MemoryPoolTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/Threading.h"
#include "Futile/Assert.h"
#include "Poco/Thread.h"
#include "Poco/Stopwatch.h"
#include <iostream>


#define private public
#include "Futile/MemoryPool.h"


using namespace Futile;


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


namespace {


struct Point
{

    Point() : x(0), y(0) {}

    Point(int inX, int inY) : x(inX), y(inY) {}

    int x;
    int y;
};

static Point * CreatePoint(void * placement, int x, int y)
{
    return new (placement) Point(x, y);
}


} // namespace


void MemoryPoolTest::testMemoryPool()
{
    std::cout << std::endl;

    const std::size_t cItemCount = 100;
    const std::size_t cByteCount = cItemCount * sizeof(Point);

    Futile::MemoryPool<Point> pool(cItemCount);

    std::cout << "Testing pool.size() " << std::flush;
    assertEqual(pool.size(), cByteCount);
    std::cout << "OK\n";

    std::cout << "Testing pool.used() " << std::flush;
    assertEqual(pool.used(), std::size_t(0));
    std::cout << "OK\n";

    std::cout << "Testing pool.available() " << std::flush;
    assertEqual(pool.available(), cItemCount);
    std::cout << "OK\n";

    std::cout << "Testing pool.acquire() " << std::flush;
    Point * point = new (pool.acquire()) Point(3, 4);
    std::cout << "OK\n";

    std::cout << "Testing pool.indexOf() " << std::flush;
    assertEqual(pool.indexOf(point), std::size_t(0));
    std::cout << "OK\n";

    assertEqual(point->x, 3);
    assertEqual(point->y, 4);

    std::cout << "Testing point->~Point() " << std::flush;
    point->~Point();
    std::cout << "\n";


    std::cout << "Testing pool.release() " << std::flush;
    pool.release(point);
    std::cout << "\n";

    std::cout << "Testing pool counts." << std::flush;
    assertEqual(pool.size(), cByteCount);
    assertEqual(pool.used(), std::size_t(0));
    assertEqual(pool.available(), cItemCount);
    std::cout << "OK\n";


    std::cout << "Testing MemoryPool::ScopedPtr " << std::flush;
    {
        typedef MemoryPool<Point>::ScopedPtr ScopedPtr;
        MemoryPool<Point> pool(cItemCount);
        assertEqual(pool.used(), 0);
        assertEqual(pool.available(), cItemCount);
        {
            ScopedPtr ptr1(pool, pool.acquireAndDefaultConstruct());
            assertEqual(pool.used(), 1);
            assertEqual(pool.available(), cItemCount - 1);

            ScopedPtr ptr2(pool, pool.acquireAndDefaultConstruct());
            assertEqual(pool.used(), 2);
            assertEqual(pool.available(), cItemCount - 2);
        }
        assertEqual(pool.used(), 0);
        assertEqual(pool.available(), cItemCount);
    }
    std::cout << "OK\n";


    {
        typedef MemoryPool<Point>::SharedPtr SharedPtr;

        std::cout << "Create the pool at most outer scope " << std::flush;
        MemoryPool<Point> pool(cItemCount);
        std::cout << "OK\n";
        {
            std::cout << "Check use count is zero before creating any shared pointers " << std::flush;
            assertEqual(pool.used(), 0);
            std::cout << "OK\n";

            std::cout << "Check use count is zero after default constructing a sharedPtr without setting it " << std::flush;
            SharedPtr outer_0(pool);
            assertEqual(pool.used(), 0);
            std::cout << "OK\n";

            std::cout << "Check default created shared ptr has a null value" << std::flush;
            assertEqual(outer_0.get(), NULL);
            std::cout << "OK\n";

            std::cout << "Create a second shared pointer in outer scope, this time its set " << std::flush;
            SharedPtr outer_1(pool, pool.acquireAndDefaultConstruct());
            assertEqual(pool.used(), 1);
            Assert(outer_1.get());
            Assert(outer_0.get() != outer_1.get());
            std::cout << "OK\n";

            {
                std::cout << "Create inner shared pointer and set " << std::flush;
                SharedPtr inner_1(pool, pool.acquireAndDefaultConstruct());
                std::cout << "OK\n";

                std::cout << "Check use count " << std::flush;
                assertEqual(pool.used(), 2);
                std::cout << "OK\n";

                std::cout << "Overwrite inner (set) pointer with outer (set) pointer " << std::flush;
                inner_1.reset(outer_1.get());
                std::cout << "OK\n";

                std::cout << "Check if use count has decremented after overwriting the outer shared ptr " << std::flush;
                assertEqual(pool.used(), 1);

                std::cout << "Overwrite outer unset pointer with inner set pointer " << std::flush;
                outer_1 = inner_1;
                std::cout << "OK\n";

                std::cout << "Verify that use count has remained unchanged " << std::flush;
                assertEqual(pool.used(), 1);
                std::cout << "OK\n";
            }
        }
        std::cout << "Verify use count zero after destroying all shared poiters " << std::flush;
        assertEqual(pool.used(), 0);
        std::cout << " OK\n";

        std::cout << "Verify available count equal to max after destroying all shared poiters " << std::flush;
        assertEqual(pool.available(), cItemCount);
        std::cout << " OK\n";
    }


    std::cout << "Testing MemoryPool filling " << std::flush;
    std::vector<Point*> points;
    for (std::size_t idx = 0; idx < cItemCount; ++idx)
    {
        Point * point = pool.acquireAndConstruct(boost::bind(CreatePoint, _1, idx, cItemCount));
        point->x = idx;
        point->y = cItemCount;

        assertEqual(pool.size(), cByteCount);
        assertEqual(pool.used(), idx + 1);
        assertEqual(pool.available(), cItemCount - idx - 1);
        assertEqual(pool.indexOf(point), idx);
        assertEqual(pool.offsetOf(point), sizeof(Point) * idx);

        points.push_back(point);
    }
    std::cout << "OK\n";

    std::cout << "Testing MemoryPool erasing " << std::flush;
    while (pool.used() > 0)
    {
        assertEqual(pool.used(), points.size());
        assertEqual(points.back()->x + 1, pool.used());
        pool.destructAndRelease(points.back());
        points.pop_back();
    }
    std::cout << "OK\n";
}


CppUnit::Test * MemoryPoolTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("MemoryPoolTest"));
    CppUnit_addTest(suite, MemoryPoolTest, testMemoryPool);
    return suite;
}
