#include "NNodeTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/Playground/NNode.h"
#include "Futile/Allocator.h"
#include "Futile/Assert.h"
#include <boost/scoped_ptr.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>


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

    GameStateNode()
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


unsigned ConvertToMB(unsigned inByteCount)
{
    return static_cast<unsigned>(0.5 + static_cast<double>(inByteCount) / (1024.0 * 1024.0));
}


unsigned ConvertToKB(unsigned inByteCount)
{
    return static_cast<unsigned>(0.5 + static_cast<double>(inByteCount) / 1024.0);
}



std::string ConvertToUnit(unsigned inByteCount)
{
    std::stringstream ss;
    if (inByteCount > 1024 * 1024)
    {
        ss << ConvertToMB(inByteCount) << " MB";
    }
    else if (inByteCount > 1024)
    {
        ss << ConvertToKB(inByteCount) << " KB";
    }
    else
    {
        ss << sizeof(inByteCount) << " B";
    }
    return ss.str();
}



template<typename Node>
void PrintInfo()
{
    std::cout << "depth: "  << Node::Depth  << ", "
              << "height: " << Node::Height << ", "
              << "size: "   << std::setw(6) << ConvertToUnit(sizeof(Node)) << std::endl;
}



} // anonymous namespace


void NNodeTest::testNode()
{
    std::cout << "\n";
    std::cout << "Size of the node: " << std::endl;
    PrintInfo<GameStateNode>();
    PrintInfo<GameStateNode::Child>();
    PrintInfo<GameStateNode::Child::Child>();
    PrintInfo<GameStateNode::Child::Child::Child>();
    PrintInfo<GameStateNode::Child::Child::Child::Child>();
    PrintInfo<GameStateNode::Child::Child::Child::Child::Child>();
    PrintInfo<GameStateNode::Child::Child::Child::Child::Child::Child>();
    PrintInfo<GameStateNode::Child::Child::Child::Child::Child::Child::Child>();
    boost::scoped_ptr<GameStateNode> test(new GameStateNode);
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