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
    Assert(grid.numRows() == 4);
    Assert(grid.numColumns() == 3);
    for (size_t r = 0; r < grid.numRows(); ++r)
    {
        for (size_t c = 0; c < grid.numColumns(); ++c)
        {
            Assert(grid.get(r, c) == 1);
        }
    }

    for (size_t r = 0; r < grid.numRows(); ++r)
    {
        for (size_t c = 0; c < grid.numColumns(); ++c)
        {
            grid.set(r, c, r*c);
            Assert(grid.get(r, c) == r*c);
        }
    }

    GenericGrid<int> grid2(grid);
    Assert(grid2.numRows() == grid.numRows());
    Assert(grid2.numColumns() == grid.numColumns());

    GenericGrid<int> grid3(4, 3);
    grid3 = grid2;
    Assert(grid3.numRows() == grid2.numRows());
    Assert(grid3.numColumns() == grid2.numColumns());

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
