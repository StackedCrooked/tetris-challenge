#ifndef BLOCKS_H
#define BLOCKS_H


#include "GenericGrid.h"
#include <map>


namespace Tetris
{

    enum BlockType
    {
        NO_BLOCK = 0,
        FIRST_BLOCK,
        I_BLOCK = FIRST_BLOCK,
        J_BLOCK,
        L_BLOCK,
        O_BLOCK,
        S_BLOCK,
        T_BLOCK,
        Z_BLOCK,
        END_BLOCK
    };
    typedef GenericGrid<BlockType> Grid;

    /**
     * Helper class that identifies a block by its type and rotation.
     * Also defines equality and smaller-than operators for a block so
     * that it can be used as the key in a key-value mapping datastructure.
     */
    class Block
    {
    public:
        Block(char inCharId, BlockType inType, int inRotation);

        // The ID in inputs.txt
        char charId() const;

        BlockType type() const;

        int rotation() const;

        const Grid & grid() const;

    private:
        char mCharId;
        BlockType mType;
        int mRotation;

    };

    bool operator< (const Block & lhs, const Block & rhs);

    const Grid & GetGrid(const Block & inBlock);
    int NumRotations(BlockType inType);
    Grid MakeBlock(int inType, int rotation);
    Grid MakeIBlock(int rotation);
    Grid MakeJBlock(int rotation);
    Grid MakeLBlock(int rotation);
    Grid MakeOBlock(int rotation);
    Grid MakeSBlock(int rotation);
    Grid MakeTBlock(int rotation);
    Grid MakeZBlock(int rotation);

    typedef std::map<Block, Grid> Grids;

} // namespace Tetris



#endif // BLOCKS_H
