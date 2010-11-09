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


    T * alloc()
    {
        return reinterpret_cast<T*>(mMemoryPool.get());
    }

    void free(T * inBuffer)
    {
        mMemoryPool.release(inBuffer);
    }

private:
    Poco::MemoryPool mMemoryPool;
};


template<class GridType>
static void TestGrid()
{
    GridType grid(3, 4);
    Assert(grid.rowCount() == 3);
    Assert(grid.columnCount() == 4);

    for (size_t r = 0; r < grid.rowCount(); ++r)
    {
        for (size_t c = 0; c < grid.columnCount(); ++c)
        {
            int count = r * grid.columnCount() + c;
            grid.set(r, c, count);
            Assert(grid.get(r, c) == count);
            count++;
        }
    }
}


template<class GridType, class T>
static void TestGridWithInitialValue(const T & inInitialValue)
{
    GridType grid(3, 4, inInitialValue);
    Assert(grid.rowCount() == 3);
    Assert(grid.columnCount() == 4);

    for (size_t r = 0; r < grid.rowCount(); ++r)
    {
        for (size_t c = 0; c < grid.columnCount(); ++c)
        {
            int count = r * grid.columnCount() + c;
            
            // Check if initial value was set.
            Assert(grid.get(r, c) == inInitialValue);

            // Set a new value and check if it can be retrieved.
            grid.set(r, c, count);
            Assert(grid.get(r, c) == count);

            count++;
        }
    }
}


void GenericGrid2Test::testAllocator_Malloc()
{
    TestGrid<Grid<int, Allocator_Malloc>>();
}


void GenericGrid2Test::testAllocator_New()
{
    TestGrid<Grid<int, Allocator_New>>();
}


void GenericGrid2Test::testAllocator_PocoMemoryPool()
{
    TestGrid<Grid<int, Allocator_PocoMemoryPool>>();
}


void GenericGrid2Test::testAllocator_Malloc_WithInitialValue()
{
    TestGridWithInitialValue<Grid<int, Allocator_Malloc>>(1);
}


void GenericGrid2Test::testAllocator_New_WithInitialValue()
{
    TestGridWithInitialValue<Grid<int, Allocator_New>>(2);
}


void GenericGrid2Test::testAllocator_PocoMemoryPool_WithInitialValue()
{
    TestGridWithInitialValue<Grid<int, Allocator_PocoMemoryPool>>(3);
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
	CppUnit_addTest(suite, GenericGrid2Test, testAllocator_Malloc);
	CppUnit_addTest(suite, GenericGrid2Test, testAllocator_New);
	CppUnit_addTest(suite, GenericGrid2Test, testAllocator_PocoMemoryPool);
	CppUnit_addTest(suite, GenericGrid2Test, testAllocator_Malloc_WithInitialValue);
	CppUnit_addTest(suite, GenericGrid2Test, testAllocator_New_WithInitialValue);
	CppUnit_addTest(suite, GenericGrid2Test, testAllocator_PocoMemoryPool_WithInitialValue);
	return suite;
}
