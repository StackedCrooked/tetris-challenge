#include "NNodeTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/Playground/NNode.h"
#include "Futile/Allocator.h"
#include "Futile/Assert.h"
#include <boost/scoped_ptr.hpp>
#include <iostream>


using namespace Futile;


namespace {


template<typename T, unsigned R, unsigned C>
struct FlatGrid
{
    static const unsigned RowCount = R;
    static const unsigned ColumnCount = C;

    FlatGrid() :
        mData()
    {
    }

    const T & get(unsigned inRow, unsigned inCol) const
    {
        return mData[inRow * ColumnCount + inCol];
    }

    T & get(unsigned inRow, unsigned inCol)
    {
        return mData[inRow * ColumnCount + inCol];
    }

    void set(const T & inValue, unsigned inRow, unsigned inCol)
    {
        mData[inRow * ColumnCount + inCol] = inValue;
    }

    typedef T Data[RowCount * ColumnCount];
    Data mData;
};


typedef FlatGrid<char, 20, 10> Grid;


struct GameState
{
    GameState() :
        mGrid(),
        mScore(0)
    {
    }

    Grid mGrid;
    int mScore;
};


struct GameStateNode : NNode<GameState, 5, 8, 0>
{
    typedef NNode<GameState, 5, 8, 0> Base;

    GameStateNode() :
        Base()
    {
    }

    const Grid & grid() const
    {
        return mData.mGrid;
    }

    Grid & grid()
    {
        return mData.mGrid;
    }

    int score() const
    {
        return mData.mScore;
    }
};



} // anonymous namespace


void NNodeTest::testNode()
{
    const unsigned cSizeMB = static_cast<int>(0.5 + (sizeof(GameStateNode) / (1024.0 * 1024.0)));
    std::cout << "The size of the Tree is  " << cSizeMB << " MB" << std::endl;
    boost::scoped_ptr<GameStateNode> theGameStateNode(new GameStateNode);
    GameStateNode & node = *theGameStateNode.get();
}


void NNodeTest::setUp()
{
}


void NNodeTest::tearDown()
{
}


NNodeTest::NNodeTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


NNodeTest::~NNodeTest()
{
}


CppUnit::Test * NNodeTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("NodeTest"));
    CppUnit_addTest(suite, NNodeTest, testNode);
    return suite;
}
