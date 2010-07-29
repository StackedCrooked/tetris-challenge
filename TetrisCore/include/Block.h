#ifndef BLOCKS_H
#define BLOCKS_H


#include "GenericGrid.h"
#include <map>


namespace Tetris
{

    enum BlockType
    {
        BlockType_Unknown = 0,              // value 0 enables us to type code like: `if (blockType)' and `if (!blockType)'
        BlockType_Begin,                    // 'begin' is value 1.
        BlockType_I = BlockType_Begin,
        BlockType_J,
        BlockType_L,
        BlockType_O,
        BlockType_S,
        BlockType_T,
        BlockType_Z,
        BlockType_End                      // 'end' is one past the last value (like stl iterators)
    };
    
    typedef GenericGrid<BlockType> Grid;

    /**
     * Represents a Tetris block.
     */
    class Block
    {
    public:
        Block(BlockType inType, int inRotation);

        int id() const;

        BlockType type() const;

        int rotation() const;

        // Get the grid associated with this block
        const Grid & grid() const;

    private:
        int mId;
        BlockType mType;
        int mRotation;
        const Grid & mGrid;
    };

} // namespace Tetris



#endif // BLOCKS_H
