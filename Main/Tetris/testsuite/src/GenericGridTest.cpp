#include "GenericGridTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/Allocator.h"
#include "Futile/GenericGrid.h"
#include "Futile/Assert.h"
#include <iostream>


using Futile::GenericGrid;
using Futile::Allocator_Malloc;
using Futile::Allocator_New;
using Futile::Allocator_Vector;


GenericGridTest::GenericGridTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


GenericGridTest::~GenericGridTest()
{
}


template< template <class> class AllocatorType >
void GenericGridTest_TestGenericGrid(const std::string & inAllocatorType)
{
	std::cout << std::endl << "Test with allocator type: " << inAllocatorType;

    GenericGrid<int, AllocatorType> grid(4, 3, 1);
    Assert(grid.rowCount() == 4);
    Assert(grid.columnCount() == 3);
    for (size_t r = 0; r < grid.rowCount(); ++r)
    {
        for (size_t c = 0; c < grid.columnCount(); ++c)
        {
            Assert(grid.get(r, c) == 1);
        }
    }

    for (size_t r = 0; r < grid.rowCount(); ++r)
    {
        for (size_t c = 0; c < grid.columnCount(); ++c)
        {
            grid.set(r, c, r*c);
            Assert(grid.get(r, c) == r*c);
        }
    }
}


template< template <class> class AllocatorType >
void GenericGridTest_TestGenericGridCopy(const std::string & inAllocatorType)
{
	std::cout << std::endl << "Test with allocator type: " << inAllocatorType;

    GenericGrid<int, AllocatorType> grid(4, 3, 1);
    Assert(grid.rowCount() == 4);
    Assert(grid.columnCount() == 3);
    for (size_t r = 0; r < grid.rowCount(); ++r)
    {
        for (size_t c = 0; c < grid.columnCount(); ++c)
        {
            Assert(grid.get(r, c) == 1);
        }
    }

    for (size_t r = 0; r < grid.rowCount(); ++r)
    {
        for (size_t c = 0; c < grid.columnCount(); ++c)
        {
            grid.set(r, c, r*c);
            Assert(grid.get(r, c) == r*c);
        }
    }

    GenericGrid<int, AllocatorType> grid2(grid);
    Assert(grid2.rowCount() == grid.rowCount());
    Assert(grid2.columnCount() == grid.columnCount());

    GenericGrid<int, AllocatorType> grid3(4, 3);
    grid3 = grid2;
    Assert(grid3.rowCount() == grid2.rowCount());
    Assert(grid3.columnCount() == grid2.columnCount());
}


void GenericGridTest::testGenericGrid()
{
	GenericGridTest_TestGenericGridCopy<Allocator_Vector>("Allocator_Vector");
	GenericGridTest_TestGenericGrid<Allocator_Malloc>("Allocator_Malloc");
	GenericGridTest_TestGenericGrid<Allocator_New>("Allocator_New");
}


void GenericGridTest::setUp()
{
}


void GenericGridTest::tearDown()
{
}


CppUnit::Test * GenericGridTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("GenericGridTest"));
	CppUnit_addTest(suite, GenericGridTest, testGenericGrid);
	return suite;
}
