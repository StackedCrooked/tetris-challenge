#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/GenericGrid.h"
#include "Tetris/Assert.h"
#include <stdexcept>


namespace Tetris
{

    class BlockImpl
    {
    public:
        BlockImpl(BlockType inType, Rotation inRotation, Row inRow, Column inColumn) :
            mType(inType),
            mRotation(inRotation.get()),
            mRow(inRow.get()),
            mColumn(inColumn.get()),
            mGrid(&GetGrid(GetBlockIdentifier(inType, inRotation.get())))
        {
        }
        BlockType mType;
        size_t mRotation;
        size_t mRow;
        size_t mColumn;
        const Grid * mGrid;
    };


    Block::Block(BlockType inType, Rotation inRotation, Row inRow, Column inColumn) :
        mImpl(new BlockImpl(inType, inRotation, inRow, inColumn))
    {
        Assert(mImpl->mRotation >= 0 && mImpl->mRotation <= 3);
    }


    Block::Block(const Block & rhs) :
        mImpl(new BlockImpl(*rhs.mImpl))
    {
    }

    
    Block & Block::operator=(const Block & rhs)
    {
        if (&rhs != this)
        {
            delete mImpl;
            mImpl = 0;
            mImpl = new BlockImpl(*rhs.mImpl);
        }
        return *this;
    }


    Block::~Block()
    {
        delete mImpl;
        mImpl = 0;
    }


    BlockType Block::type() const
    {
        return mImpl->mType;
    }


    size_t Block::rotation() const
    {
        return mImpl->mRotation;
    }


    size_t Block::numRotations() const
    {
        return GetBlockRotationCount(mImpl->mType);
    }


    const Grid & Block::grid() const
    {
        return *mImpl->mGrid;
    }


    size_t Block::row() const
    {
        return mImpl->mRow;
    }


    size_t Block::column() const
    {
        return mImpl->mColumn;
    }


    void Block::setRow(size_t inRow)
    {
        mImpl->mRow = inRow;
    }


    void Block::setColumn(size_t inColumn)
    {
        mImpl->mColumn = inColumn;
    }


    void Block::setRotation(size_t inRotation)
    {
        mImpl->mRotation = inRotation % GetBlockRotationCount(mImpl->mType);
        mImpl->mGrid = &GetGrid(GetBlockIdentifier(mImpl->mType, mImpl->mRotation));
    }


    void Block::rotate()
    {
        setRotation((mImpl->mRotation + 1) % GetBlockRotationCount(mImpl->mType));
    }


    size_t GetBlockIdentifier(BlockType inType, size_t inRotation)
    {
        Assert(inType >= BlockType_Begin || inType < BlockType_End);
        // Max 4 rotations.
        return 4 * static_cast<int>(inType - 1) + inRotation;
    }


    size_t GetBlockRotationCount(BlockType inType)
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


    size_t GetBlockPositionCount(BlockType inType, size_t inNumColumns)
    {
        size_t result = 0;
        size_t numRotations = GetBlockRotationCount(inType);
        for (size_t idx = 0; idx != numRotations; ++idx)
        {
            const Grid & grid = GetGrid(GetBlockIdentifier(inType, idx));
            result += inNumColumns - grid.numColumns() + 1;
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

    Grid GetOGrid(int rotation)
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
