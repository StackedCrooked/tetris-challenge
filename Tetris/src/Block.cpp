#include "Tetris/Config.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Assert.h"
#include <stdexcept>


using Futile::Create;
using Futile::CreatePoly;


namespace Tetris {


class BlockImpl
{
public:
    BlockImpl(BlockType inType, Rotation inRotation, Row inRow, Column inColumn);

    std::auto_ptr<BlockImpl> clone() const;

    BlockType type() const;

    std::size_t rotation() const;

    std::size_t numRotations() const;

    const Grid & grid() const;

    std::size_t row() const;

    std::size_t column() const;

    std::size_t rowCount() const;

    std::size_t columnCount() const;

    void rotate();

    void setRow(std::size_t inRow);

    void setColumn(std::size_t inColumn);

    void setRotation(std::size_t inRotation);

private:
    BlockType mType;
    std::size_t mRotation;
    std::size_t mRow;
    std::size_t mColumn;
    const Grid * mGrid;
};


BlockImpl::BlockImpl(BlockType inType, Rotation inRotation, Row inRow, Column inColumn) :
    mType(inType),
    mRotation(inRotation),
    mRow(inRow),
    mColumn(inColumn),
    mGrid(&GetGrid(GetBlockIdentifier(inType, inRotation)))
{
    Assert(mRotation >= 0 && mRotation <= 3);
}


std::auto_ptr<BlockImpl> BlockImpl::clone() const
{
    return Create<BlockImpl>(*this);
}


BlockType BlockImpl::type() const
{
    return mType;
}


std::size_t BlockImpl::rotation() const
{
    return mRotation;
}


std::size_t BlockImpl::numRotations() const
{
    return GetBlockRotationCount(mType);
}


const Grid & BlockImpl::grid() const
{
    return *mGrid;
}


std::size_t BlockImpl::row() const
{
    return mRow;
}


std::size_t BlockImpl::column() const
{
    return mColumn;
}


std::size_t BlockImpl::rowCount() const
{
    return mGrid->rowCount();
}


std::size_t BlockImpl::columnCount() const
{
    return mGrid->columnCount();
}


void BlockImpl::setRow(std::size_t inRow)
{
    mRow = inRow;
}


void BlockImpl::setColumn(std::size_t inColumn)
{
    mColumn = inColumn;
}


void BlockImpl::setRotation(std::size_t inRotation)
{
    mRotation = inRotation % GetBlockRotationCount(mType);
    mGrid = &GetGrid(GetBlockIdentifier(mType, mRotation));
}


void BlockImpl::rotate()
{
    setRotation((mRotation + 1) % GetBlockRotationCount(mType));
}


Block::Block(BlockType inType, Rotation inRotation, Row inRow, Column inColumn) :
    mImpl(Create<BlockImpl>(inType, inRotation, inRow, inColumn).release())
{
}


Block::Block(const Block & rhs) :
    mImpl(rhs.mImpl->clone().release())
{
}


Block & Block::operator=(const Block & rhs)
{
    if (&rhs != this)
    {
        delete mImpl;
        mImpl = Create<BlockImpl>(*rhs.mImpl).release();
    }
    return *this;
}


Block::~Block()
{
    delete mImpl;
    mImpl = 0;
}


int Block::identification() const
{
    return GetBlockIdentifier(type(), rotation());
}


BlockType Block::type() const
{
    return mImpl->type();
}


std::size_t Block::rotation() const
{
    return mImpl->rotation();
}


std::size_t Block::numRotations() const
{
    return mImpl->numRotations();
}


const Grid & Block::grid() const
{
    return mImpl->grid();
}


std::size_t Block::row() const
{
    return mImpl->row();
}


std::size_t Block::column() const
{
    return mImpl->column();
}


std::size_t Block::rowCount() const
{
    return mImpl->rowCount();
}


std::size_t Block::columnCount() const
{
    return mImpl->columnCount();
}


void Block::setRow(std::size_t inRow)
{
    mImpl->setRow(inRow);
}


void Block::setColumn(std::size_t inColumn)
{
    mImpl->setColumn(inColumn);
}


void Block::setRotation(std::size_t inRotation)
{
    mImpl->setRotation(inRotation);
}


void Block::rotate()
{
    mImpl->rotate();
}


int GetBlockIdentifier(BlockType inType, int inRotation)
{
    Assert(inType >= BlockType_Begin || inType < BlockType_End);
    // Max 4 rotations.
    return 4 * static_cast<int>(inType - 1) + inRotation;
}


std::size_t GetBlockRotationCount(BlockType inType)
{
    Assert(inType >= BlockType_Begin || inType < BlockType_End);
    static const int fRotationCounts[] =
    {
        2, // BlockType_I
        4, // BlockType_J
        4, // BlockType_L
        1, // BlockType_O
        2, // BlockType_S
        4, // BlockType_T
        2  // BlockType_Z
    };
    return fRotationCounts[static_cast<int>(inType) - 1];
}


std::size_t GetBlockPositionCount(BlockType inType, std::size_t inNumColumns)
{
    std::size_t result = 0;
    std::size_t numRotations = GetBlockRotationCount(inType);
    for (std::size_t idx = 0; idx != numRotations; ++idx)
    {
        const Grid & grid = GetGrid(GetBlockIdentifier(inType, idx));
        result += inNumColumns - grid.columnCount() + 1;
    }
    return result;
}


Grid GetIGrid(int rotation);
Grid GetJGrid(int rotation);
Grid GetLGrid(int rotation);
Grid GetOGrid(int rotation);
Grid GetSGrid(int rotation);
Grid GetTGrid(int rotation);
Grid GetZGrid(int rotation);


const Grid & GetGrid(int inId)
{
    if (inId < 0 || inId >= 28)
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


Grid GetIGrid(int rotation)
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


Grid GetJGrid(int rotation)
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

Grid GetLGrid(int rotation)
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

Grid GetOGrid(int) // rotation is not relevant
{
    Grid grid(2, 2, BlockType_Nil);
    grid.set(0, 0, BlockType_O);
    grid.set(0, 1, BlockType_O);
    grid.set(1, 0, BlockType_O);
    grid.set(1, 1, BlockType_O);
    return grid;
}

Grid GetSGrid(int rotation)
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

Grid GetTGrid(int rotation)
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

Grid GetZGrid(int rotation)
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
