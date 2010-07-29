#include "Block.h"
#include <assert.h>


namespace Tetris
{

    static Grids sGrids;


    const Grid & GetGrid(const Block & inBlock)
    {
        Grids::iterator it = sGrids.find(inBlock);
        if (it == sGrids.end())
        {
            sGrids.insert(std::make_pair(inBlock, MakeBlock(inBlock.type(), inBlock.rotation())));
            it = sGrids.find(inBlock);
        }
        return it->second;
    }



    int NumRotations(BlockType inType)
    {
        switch (inType)
        {
            case I_BLOCK:
            {
                return 2;
            }
            case J_BLOCK:
            {
                return 4;
            }
            case L_BLOCK:
            {
                return 4;
            }
            case O_BLOCK:
            {
                return 1;
            }
            case S_BLOCK:
            {
                return 2;
            }
            case T_BLOCK:
            {
                return 4;
            }
            case Z_BLOCK:
            {
                return 2;
            }
            default:
            {
                assert(!"No valid block type given.");
                return 1;
            }
        }
    }


    Block::Block(char inCharId, BlockType inType, int inRotation) :
        mCharId(inCharId),
        mType(inType),
        mRotation(inRotation)
    {
    }


    char Block::charId() const
    {
        return mCharId;
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
        return GetGrid(*this);
    }


    bool operator< (const Block & lhs, const Block & rhs)
    {
        if (lhs.type() != rhs.type())
        {
            return lhs.type() < rhs.type();
        }
        else
        {
            return lhs.rotation() < rhs.rotation();
        }
    }


    Grid MakeBlock(int inType, int rotation)
    {
        if (inType == I_BLOCK)
        {
            return MakeIBlock(rotation);
        }
        else if (inType == J_BLOCK)
        {
            return MakeJBlock(rotation);
        }
        else if (inType == L_BLOCK)
        {
            return MakeLBlock(rotation);
        }
        else if (inType == O_BLOCK)
        {
            return MakeOBlock(rotation);
        }
        else if (inType == S_BLOCK)
        {
            return MakeSBlock(rotation);
        }
        else if (inType == T_BLOCK)
        {
            return MakeTBlock(rotation);
        }
        else if (inType == Z_BLOCK)
        {
            return MakeZBlock(rotation);
        }
        else
        {
            assert(!"No valid block type given!");
        }

        // if we get here something is wrong, however we have to return something
        static Grid failBlock(4, 4, NO_BLOCK);
        return failBlock;
    }


    Grid MakeIBlock(int rotation)
    {
        if (rotation%2 == 0)
        {
            Grid block(1, 4, NO_BLOCK);
            block.set(0, 0, I_BLOCK);
            block.set(0, 1, I_BLOCK);
            block.set(0, 2, I_BLOCK);
            block.set(0, 3, I_BLOCK);
            return block;
        }
        else
        {
            Grid block(4, 1, NO_BLOCK);
            block.set(0, 0, I_BLOCK);
            block.set(1, 0, I_BLOCK);
            block.set(2, 0, I_BLOCK);
            block.set(3, 0, I_BLOCK);
            return block;
        }
    }

    Grid MakeJBlock(int rotation)
    {
        if (rotation%4 == 0)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 0, J_BLOCK);
            block.set(1, 0, J_BLOCK);
            block.set(1, 1, J_BLOCK);
            block.set(1, 2, J_BLOCK);
            return block;
        }
        else if (rotation%4 == 1)
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 0, J_BLOCK);
            block.set(0, 1, J_BLOCK);
            block.set(1, 0, J_BLOCK);
            block.set(2, 0, J_BLOCK);
            return block;
        }
        else if (rotation%4 == 2)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 0, J_BLOCK);
            block.set(0, 1, J_BLOCK);
            block.set(0, 2, J_BLOCK);
            block.set(1, 2, J_BLOCK);
            return block;
        }
        else
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 1, J_BLOCK);
            block.set(1, 1, J_BLOCK);
            block.set(2, 0, J_BLOCK);
            block.set(2, 1, J_BLOCK);
            return block;
        }
    }

    Grid MakeLBlock(int rotation)
    {
        if (rotation%4 == 0)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 2, L_BLOCK);
            block.set(1, 0, L_BLOCK);
            block.set(1, 1, L_BLOCK);
            block.set(1, 2, L_BLOCK);
            return block;
        }
        else if (rotation%4 == 1)
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 0, L_BLOCK);
            block.set(1, 0, L_BLOCK);
            block.set(2, 0, L_BLOCK);
            block.set(2, 1, L_BLOCK);
            return block;
        }
        else if (rotation%4 == 2)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 0, L_BLOCK);
            block.set(0, 1, L_BLOCK);
            block.set(0, 2, L_BLOCK);
            block.set(1, 0, L_BLOCK);
            return block;
        }
        else
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 0, L_BLOCK);
            block.set(0, 1, L_BLOCK);
            block.set(1, 1, L_BLOCK);
            block.set(2, 1, L_BLOCK);
            return block;
        }
    }

    Grid MakeOBlock(int rotation)
    {
        Grid block(2, 2, NO_BLOCK);
        block.set(0, 0, O_BLOCK);
        block.set(0, 1, O_BLOCK);
        block.set(1, 0, O_BLOCK);
        block.set(1, 1, O_BLOCK);
        return block;
    }

    Grid MakeSBlock(int rotation)
    {
        if (rotation%2 == 0)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 1, S_BLOCK);
            block.set(0, 2, S_BLOCK);
            block.set(1, 0, S_BLOCK);
            block.set(1, 1, S_BLOCK);
            return block;
        }
        else
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 0, S_BLOCK);
            block.set(1, 0, S_BLOCK);
            block.set(1, 1, S_BLOCK);
            block.set(2, 1, S_BLOCK);
            return block;
        }
    }

    Grid MakeTBlock(int rotation)
    {
        if (rotation%4 == 0)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 1, T_BLOCK);
            block.set(1, 0, T_BLOCK);
            block.set(1, 1, T_BLOCK);
            block.set(1, 2, T_BLOCK);
            return block;
        }
        else if (rotation%4 == 1)
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 0, T_BLOCK);
            block.set(1, 0, T_BLOCK);
            block.set(1, 1, T_BLOCK);
            block.set(2, 0, T_BLOCK);
            return block;
        }
        else if (rotation%4 == 2)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 0, T_BLOCK);
            block.set(0, 1, T_BLOCK);
            block.set(0, 2, T_BLOCK);
            block.set(1, 1, T_BLOCK);
            return block;
        }
        else
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 1, T_BLOCK);
            block.set(1, 0, T_BLOCK);
            block.set(1, 1, T_BLOCK);
            block.set(2, 1, T_BLOCK);
            return block;
        }
    }

    Grid MakeZBlock(int rotation)
    {
        if (rotation%2 == 0)
        {
            Grid block(2, 3, NO_BLOCK);
            block.set(0, 0, Z_BLOCK);
            block.set(0, 1, Z_BLOCK);
            block.set(1, 1, Z_BLOCK);
            block.set(1, 2, Z_BLOCK);
            return block;
        }
        else
        {
            Grid block(3, 2, NO_BLOCK);
            block.set(0, 1, Z_BLOCK);
            block.set(1, 0, Z_BLOCK);
            block.set(1, 1, Z_BLOCK);
            block.set(2, 0, Z_BLOCK);
            return block;
        }
    }

} // namespace Tetris