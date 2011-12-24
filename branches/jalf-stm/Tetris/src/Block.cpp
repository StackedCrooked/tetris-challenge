#include "Tetris/Block.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Assert.h"
#include <stdexcept>


namespace Tetris {


using namespace Futile;


Block::Block(BlockType inType, Rotation inRotation, Row inRow, Column inColumn) :
    mType(inType),
    mRotation(inRotation),
    mRow(inRow),
    mColumn(inColumn),
    mGrid(&GetGrid(GetBlockIdentifier(inType, inRotation)))
{
    Assert(mRotation <= 3);
}


unsigned Block::identification() const
{
    return GetBlockIdentifier(type(), rotation());
}


BlockType Block::type() const
{
    return mType;
}


unsigned Block::rotation() const
{
    return mRotation;
}


unsigned Block::rotationCount() const
{
    return GetBlockRotationCount(mType);
}


unsigned Block::row() const
{
    return mRow;
}


unsigned Block::column() const
{
    return mColumn;
}


unsigned Block::rowCount() const
{
    return mGrid->rowCount();
}


unsigned Block::columnCount() const
{
    return mGrid->columnCount();
}


void Block::setRow(unsigned inRow)
{
    mRow = inRow;
}


void Block::setColumn(unsigned inColumn)
{
    mColumn = inColumn;
}


void Block::setRotation(unsigned inRotation)
{
    mRotation = inRotation % GetBlockRotationCount(mType);
    mGrid = &GetGrid(GetBlockIdentifier(mType, mRotation));
}


void Block::rotate()
{
    setRotation((mRotation + 1) % GetBlockRotationCount(mType));
}


unsigned GetBlockIdentifier(BlockType inType, unsigned inRotation)
{
    Assert(inType >= BlockType_Begin || inType < BlockType_End);
    // Max 4 rotations.
    return 4 * static_cast<unsigned>(inType - 1) + inRotation;
}


unsigned GetBlockRotationCount(BlockType inType)
{
    Assert(inType >= BlockType_Begin || inType < BlockType_End);
    static const unsigned fRotationCounts[] =
    {
        2, // BlockType_I
        4, // BlockType_J
        4, // BlockType_L
        1, // BlockType_O
        2, // BlockType_S
        4, // BlockType_T
        2  // BlockType_Z
    };
    return fRotationCounts[static_cast<unsigned>(inType) - 1];
}


unsigned GetBlockPositionCount(BlockType inType, unsigned inNumColumns)
{
    unsigned result = 0;
    unsigned numRotations = GetBlockRotationCount(inType);
    for (unsigned idx = 0; idx != numRotations; ++idx)
    {
        const Grid & grid = GetGrid(GetBlockIdentifier(inType, idx));
        result += inNumColumns - grid.columnCount() + 1;
    }
    return result;
}


Grid GetIGrid(unsigned rotation);
Grid GetJGrid(unsigned rotation);
Grid GetLGrid(unsigned rotation);
Grid GetOGrid(unsigned rotation);
Grid GetSGrid(unsigned rotation);
Grid GetTGrid(unsigned rotation);
Grid GetZGrid(unsigned rotation);


const Grid & GetGrid(unsigned inId)
{
    if (inId >= 28)
    {
        throw std::logic_error("Invalid block identifier.");
    }

    static Grid fBlocks[] =
    {
        Grid(GetIGrid(0)),
        Grid(GetIGrid(1)),
        Grid(GetIGrid(2)),
        Grid(GetIGrid(3)),

        Grid(GetJGrid(0)),
        Grid(GetJGrid(1)),
        Grid(GetJGrid(2)),
        Grid(GetJGrid(3)),

        Grid(GetLGrid(0)),
        Grid(GetLGrid(1)),
        Grid(GetLGrid(2)),
        Grid(GetLGrid(3)),

        Grid(GetOGrid(0)),
        Grid(GetOGrid(1)),
        Grid(GetOGrid(2)),
        Grid(GetOGrid(3)),

        Grid(GetSGrid(0)),
        Grid(GetSGrid(1)),
        Grid(GetSGrid(2)),
        Grid(GetSGrid(3)),

        Grid(GetTGrid(0)),
        Grid(GetTGrid(1)),
        Grid(GetTGrid(2)),
        Grid(GetTGrid(3)),

        Grid(GetZGrid(0)),
        Grid(GetZGrid(1)),
        Grid(GetZGrid(2)),
        Grid(GetZGrid(3))
    };
    return fBlocks[inId];
}


Grid GetIGrid(unsigned rotation)
{
    if (rotation%2 == 0)
    {
        Grid grid(1, 4, BlockType_Nil);
        grid.set(0, 0, BlockType_I);
        grid.set(0, 1, BlockType_I);
        grid.set(0, 2, BlockType_I);
        grid.set(0, 3, BlockType_I);
        return grid;
    }
    else
    {
        Grid grid(4, 1, BlockType_Nil);
        grid.set(0, 0, BlockType_I);
        grid.set(1, 0, BlockType_I);
        grid.set(2, 0, BlockType_I);
        grid.set(3, 0, BlockType_I);
        return grid;
    }
}


Grid GetJGrid(unsigned rotation)
{
    if (rotation%4 == 0)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 0, BlockType_J);
        grid.set(1, 0, BlockType_J);
        grid.set(1, 1, BlockType_J);
        grid.set(1, 2, BlockType_J);
        return grid;
    }
    else if (rotation%4 == 1)
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 0, BlockType_J);
        grid.set(0, 1, BlockType_J);
        grid.set(1, 0, BlockType_J);
        grid.set(2, 0, BlockType_J);
        return grid;
    }
    else if (rotation%4 == 2)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 0, BlockType_J);
        grid.set(0, 1, BlockType_J);
        grid.set(0, 2, BlockType_J);
        grid.set(1, 2, BlockType_J);
        return grid;
    }
    else
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 1, BlockType_J);
        grid.set(1, 1, BlockType_J);
        grid.set(2, 0, BlockType_J);
        grid.set(2, 1, BlockType_J);
        return grid;
    }
}

Grid GetLGrid(unsigned rotation)
{
    if (rotation%4 == 0)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 2, BlockType_L);
        grid.set(1, 0, BlockType_L);
        grid.set(1, 1, BlockType_L);
        grid.set(1, 2, BlockType_L);
        return grid;
    }
    else if (rotation%4 == 1)
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 0, BlockType_L);
        grid.set(1, 0, BlockType_L);
        grid.set(2, 0, BlockType_L);
        grid.set(2, 1, BlockType_L);
        return grid;
    }
    else if (rotation%4 == 2)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 0, BlockType_L);
        grid.set(0, 1, BlockType_L);
        grid.set(0, 2, BlockType_L);
        grid.set(1, 0, BlockType_L);
        return grid;
    }
    else
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 0, BlockType_L);
        grid.set(0, 1, BlockType_L);
        grid.set(1, 1, BlockType_L);
        grid.set(2, 1, BlockType_L);
        return grid;
    }
}

Grid GetOGrid(unsigned) // rotation is not relevant
{
    Grid grid(2, 2, BlockType_Nil);
    grid.set(0, 0, BlockType_O);
    grid.set(0, 1, BlockType_O);
    grid.set(1, 0, BlockType_O);
    grid.set(1, 1, BlockType_O);
    return grid;
}

Grid GetSGrid(unsigned rotation)
{
    if (rotation%2 == 0)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 1, BlockType_S);
        grid.set(0, 2, BlockType_S);
        grid.set(1, 0, BlockType_S);
        grid.set(1, 1, BlockType_S);
        return grid;
    }
    else
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 0, BlockType_S);
        grid.set(1, 0, BlockType_S);
        grid.set(1, 1, BlockType_S);
        grid.set(2, 1, BlockType_S);
        return grid;
    }
}

Grid GetTGrid(unsigned rotation)
{
    if (rotation%4 == 0)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 1, BlockType_T);
        grid.set(1, 0, BlockType_T);
        grid.set(1, 1, BlockType_T);
        grid.set(1, 2, BlockType_T);
        return grid;
    }
    else if (rotation%4 == 1)
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 0, BlockType_T);
        grid.set(1, 0, BlockType_T);
        grid.set(1, 1, BlockType_T);
        grid.set(2, 0, BlockType_T);
        return grid;
    }
    else if (rotation%4 == 2)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 0, BlockType_T);
        grid.set(0, 1, BlockType_T);
        grid.set(0, 2, BlockType_T);
        grid.set(1, 1, BlockType_T);
        return grid;
    }
    else
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 1, BlockType_T);
        grid.set(1, 0, BlockType_T);
        grid.set(1, 1, BlockType_T);
        grid.set(2, 1, BlockType_T);
        return grid;
    }
}

Grid GetZGrid(unsigned rotation)
{
    if (rotation%2 == 0)
    {
        Grid grid(2, 3, BlockType_Nil);
        grid.set(0, 0, BlockType_Z);
        grid.set(0, 1, BlockType_Z);
        grid.set(1, 1, BlockType_Z);
        grid.set(1, 2, BlockType_Z);
        return grid;
    }
    else
    {
        Grid grid(3, 2, BlockType_Nil);
        grid.set(0, 1, BlockType_Z);
        grid.set(1, 0, BlockType_Z);
        grid.set(1, 1, BlockType_Z);
        grid.set(2, 0, BlockType_Z);
        return grid;
    }
}


} // namespace Tetris
