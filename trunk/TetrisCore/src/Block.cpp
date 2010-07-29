#include "Block.h"
#include <stdexcept>
#include <assert.h>


namespace Tetris
{
    
    // Returns the number of rotations that a block can have.
    int GetBlockRotationCount(BlockType inType);
    
    // Calculates the block identifier using the formula:
    //   BlockType_End * inType + inRotation
    int GetBlockIdentifier(BlockType inType, int inRotation);
    
    // Gets the Grid object that is associated with a block identifier
    const Grid & GetGrid(int inBlockIdentifier);
    Grid GetIGrid(int rotation);
    Grid GetJGrid(int rotation);
    Grid GetLGrid(int rotation);
    Grid GetOGrid(int rotation);
    Grid GetSGrid(int rotation);
    Grid GetTGrid(int rotation);
    Grid GetZGrid(int rotation);



    Block::Block(BlockType inType, int inRotation) :
        mId(GetBlockIdentifier(inType, inRotation)),
        mType(inType),
        mRotation(inRotation),
        mGrid(GetGrid(mId))
    {
        if (mRotation < 0 || mRotation > 3)
        {
            throw std::logic_error("Rotation must have value 0, 1, 2 or 3.");
        }
    }


    int Block::id() const
    {
        return mId;
    }


    BlockType Block::type() const
    {
        return mType;
    }


    int Block::rotation() const
    {
        return mRotation;
    }


    const Grid & Block::grid() const
    {
        return mGrid;
    }
 

    int GetBlockIdentifier(BlockType inType, int inRotation)
    {
        if (inType < BlockType_Begin || inType >= BlockType_End)
        {
            throw std::logic_error("Invalid block type.");
        }
        return static_cast<int>(BlockType_End - 1) * static_cast<int>(inType - 1) + inRotation;
    }


    int GetBlockRotationCount(BlockType inType)
    {
        if (inType < BlockType_Begin || inType >= BlockType_End)
        {
            throw std::logic_error("Invalid block type.");
        }
        static const int fRotationCounts[] = {2, 4, 4, 1, 2, 4, 2};
        return fRotationCounts[static_cast<int>(inType) - 1];
    }


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
            Grid grid(1, 4, BlockType_Unknown);
            grid.set(0, 0, BlockType_I);
            grid.set(0, 1, BlockType_I);
            grid.set(0, 2, BlockType_I);
            grid.set(0, 3, BlockType_I);
            return grid;
        }
        else
        {
            Grid grid(4, 1, BlockType_Unknown);
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
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 0, BlockType_J);
            grid.set(1, 0, BlockType_J);
            grid.set(1, 1, BlockType_J);
            grid.set(1, 2, BlockType_J);
            return grid;
        }
        else if (rotation%4 == 1)
        {
            Grid grid(3, 2, BlockType_Unknown);
            grid.set(0, 0, BlockType_J);
            grid.set(0, 1, BlockType_J);
            grid.set(1, 0, BlockType_J);
            grid.set(2, 0, BlockType_J);
            return grid;
        }
        else if (rotation%4 == 2)
        {
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 0, BlockType_J);
            grid.set(0, 1, BlockType_J);
            grid.set(0, 2, BlockType_J);
            grid.set(1, 2, BlockType_J);
            return grid;
        }
        else
        {
            Grid grid(3, 2, BlockType_Unknown);
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
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 2, BlockType_L);
            grid.set(1, 0, BlockType_L);
            grid.set(1, 1, BlockType_L);
            grid.set(1, 2, BlockType_L);
            return grid;
        }
        else if (rotation%4 == 1)
        {
            Grid grid(3, 2, BlockType_Unknown);
            grid.set(0, 0, BlockType_L);
            grid.set(1, 0, BlockType_L);
            grid.set(2, 0, BlockType_L);
            grid.set(2, 1, BlockType_L);
            return grid;
        }
        else if (rotation%4 == 2)
        {
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 0, BlockType_L);
            grid.set(0, 1, BlockType_L);
            grid.set(0, 2, BlockType_L);
            grid.set(1, 0, BlockType_L);
            return grid;
        }
        else
        {
            Grid grid(3, 2, BlockType_Unknown);
            grid.set(0, 0, BlockType_L);
            grid.set(0, 1, BlockType_L);
            grid.set(1, 1, BlockType_L);
            grid.set(2, 1, BlockType_L);
            return grid;
        }
    }

    Grid GetOGrid(int rotation)
    {
        Grid grid(2, 2, BlockType_Unknown);
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
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 1, BlockType_S);
            grid.set(0, 2, BlockType_S);
            grid.set(1, 0, BlockType_S);
            grid.set(1, 1, BlockType_S);
            return grid;
        }
        else
        {
            Grid grid(3, 2, BlockType_Unknown);
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
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 1, BlockType_T);
            grid.set(1, 0, BlockType_T);
            grid.set(1, 1, BlockType_T);
            grid.set(1, 2, BlockType_T);
            return grid;
        }
        else if (rotation%4 == 1)
        {
            Grid grid(3, 2, BlockType_Unknown);
            grid.set(0, 0, BlockType_T);
            grid.set(1, 0, BlockType_T);
            grid.set(1, 1, BlockType_T);
            grid.set(2, 0, BlockType_T);
            return grid;
        }
        else if (rotation%4 == 2)
        {
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 0, BlockType_T);
            grid.set(0, 1, BlockType_T);
            grid.set(0, 2, BlockType_T);
            grid.set(1, 1, BlockType_T);
            return grid;
        }
        else
        {
            Grid grid(3, 2, BlockType_Unknown);
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
            Grid grid(2, 3, BlockType_Unknown);
            grid.set(0, 0, BlockType_Z);
            grid.set(0, 1, BlockType_Z);
            grid.set(1, 1, BlockType_Z);
            grid.set(1, 2, BlockType_Z);
            return grid;
        }
        else
        {
            Grid grid(3, 2, BlockType_Unknown);
            grid.set(0, 1, BlockType_Z);
            grid.set(1, 0, BlockType_Z);
            grid.set(1, 1, BlockType_Z);
            grid.set(2, 0, BlockType_Z);
            return grid;
        }
    }

} // namespace Tetris