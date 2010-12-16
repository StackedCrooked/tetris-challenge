#include "GenericGridTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/GenericGrid.h"
#include "Tetris/Assert.h"
#include <iostream>


using namespace Tetris;


GenericGridTest::GenericGridTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


GenericGridTest::~GenericGridTest()
{
}


void GenericGridTest::testGenericGrid()
{
    GenericGrid<int> grid(4, 3, 1);
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

    GenericGrid<int> grid2(grid);
    Assert(grid2.rowCount() == grid.rowCount());
    Assert(grid2.columnCount() == grid.columnCount());

    GenericGrid<int> grid3(4, 3);
    grid3 = grid2;
    Assert(grid3.rowCount() == grid2.rowCount());
    Assert(grid3.columnCount() == grid2.columnCount());

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
