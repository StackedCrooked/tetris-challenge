#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED


#include "BlockType.h"
#include "Grid.h"


namespace Tetris
{

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
        ActiveBlock(std::auto_ptr<Block> inBlock, size_t inRow, size_t inColumn);

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
        std::auto_ptr<Block> mBlock;
        size_t mRow;
        size_t mColumn;
    };

} // namespace Tetris



#endif // BLOCK_H_INCLUDED
