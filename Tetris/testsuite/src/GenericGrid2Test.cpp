#include "GenericGrid2Test.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/GenericGrid2.h"
#include "Tetris/Assert.h"
#include "Poco/MemoryPool.h"
#include <iostream>


using namespace Tetris;


GenericGrid2Test::GenericGrid2Test(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


GenericGrid2Test::~GenericGrid2Test()
{
}

template<class T>
class Allocator_PocoMemoryPool
{
public:
    Allocator_PocoMemoryPool(size_t inSize) :
        mMemoryPool(sizeof(T) * inSize)
    {
    }


    T * Alloc()
    {
        return reinterpret_cast<T*>(mMemoryPool.get());
    }

    void Free(T * inBuffer)
    {
        mMemoryPool.release(inBuffer);
    }

private:
    Poco::MemoryPool mMemoryPool;
};


template<class T, template <class> class Allocator>
static void TestGrid()
{
    Grid<T, Allocator> grid(3, 4);
    assert(grid.rowCount() == 3);
    assert(grid.columnCount() == 4);

    for (size_t r = 0; r < grid.rowCount(); ++r)
    {
        for (size_t c = 0; c < grid.columnCount(); ++c)
        {
            int count = r * grid.columnCount() + c;
            grid.set(r, c, count);
            assert(grid.get(r, c) == count);
            count++;
        }
    }
}


void GenericGrid2Test::test()
{
    TestGrid<int, Allocator_Malloc>();
    TestGrid<int, Allocator_New>();
    TestGrid<int, Allocator_PocoMemoryPool>();
}


void GenericGrid2Test::testAll()
{
    test();
}


void GenericGrid2Test::setUp()
{
}


void GenericGrid2Test::tearDown()
{
}


CppUnit::Test * GenericGrid2Test::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("GenericGrid2Test"));
	CppUnit_addTest(suite, GenericGrid2Test, testAll);
	return suite;
}
