#ifndef TETRIS_BLOCK_H_INCLUDED
#define TETRIS_BLOCK_H_INCLUDED


#include "Tetris/Grid.h"
#include "Tetris/TypedWrapper.h"
#include <memory>


namespace Tetris
{

    // Generate the typesafe wrapper classes
    Tetris_TypedWrapper(Rotation, size_t);
    Tetris_TypedWrapper(Row, size_t);
    Tetris_TypedWrapper(Column, size_t);


    // Forward declarations.
    class BlockImpl;
    typedef char BlockType;


    /**
     * Represents a Tetris block.
     */
    class Block
    {
    public:
        Block(BlockType inType, Rotation inRotation, Row inRow, Column inColumn);

        Block(const Block & inBlock);

        Block & operator=(const Block & inBlock);

        ~Block();

        BlockType type() const;

        // Get the grid associated with this block
        const Grid & grid() const;

        size_t row() const;

        size_t rowCount() const;

        size_t column() const;

        size_t columnCount() const;

        size_t rotation() const;

        size_t numRotations() const;

        void rotate();

        void setRow(size_t inRow);

        void setColumn(size_t inColumn);

        void setRotation(size_t inRotation);

    private:
        BlockImpl * mImpl;
    };


    size_t GetBlockRotationCount(BlockType inType);

    // Returns the number possible combinations of rotations and position to
    // place a certain block in a grid that has a given a number of columns.
    size_t GetBlockPositionCount(BlockType inType, size_t inNumColumns);

    size_t GetBlockIdentifier(BlockType inType, size_t inRotation);

    // Gets the Grid object that is associated with a block identifier
    const Grid & GetGrid(int inBlockIdentifier);


} // namespace Tetris


#endif // BLOCK_H_INCLUDED
