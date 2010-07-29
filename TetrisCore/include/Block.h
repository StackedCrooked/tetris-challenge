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
     * Represents an (active) Tetris block.
     */
    class Block
    {
    public:
        Block(BlockType inType, int inRotation);

        Block(BlockType inType, int inRotation, size_t inRow, size_t inColumn);

        int id() const;

        BlockType type() const;

        int rotation() const;

        // Get the grid associated with this block
        const Grid & grid() const;

        size_t row() const;

        size_t column() const;

    private:
        int mId;
        BlockType mType;
        int mRotation;
        size_t mRow;
        size_t mColumn;
        const Grid & mGrid;
    };

} // namespace Tetris



#endif // BLOCKS_H
