#ifndef BLOCKS_H
#define BLOCKS_H


#include "GenericGrid.h"
#include <boost/scoped_ptr.hpp>
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

    const size_t cBlockTypeCount = static_cast<int>(BlockType_End) - 1;
    
    typedef GenericGrid<BlockType> Grid;

    /**
     * Represents an (active) Tetris block.
     * Non-copyable
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


    class ActiveBlock
    {
    public:
        // Block and it's position in the Grid
        ActiveBlock(const Block & inBlock, size_t inRow, size_t inColumn);

        const Block & block() const;

        size_t row() const;

        size_t column() const;

        size_t rotation() const;

        void moveLeft();

        void moveRight();

        void moveUp();

        void moveDown();

        void setRow(size_t inRow);

        void setColumn(size_t inColumn);

        void setRotation(size_t inRotation);

    private:
        boost::scoped_ptr<Block> mBlock;
        size_t mRow;
        size_t mColumn;
    };

} // namespace Tetris



#endif // BLOCKS_H
