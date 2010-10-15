#ifndef TETRIS_BLOCK_H_INCLUDED
#define TETRIS_BLOCK_H_INCLUDED


#include "Tetris/Tetris.h"


namespace Tetris
{
    
    // Forward declaration of the BlockType 'enum'
    Tetris_DeclareEnum(BlockType);

    // Forward declaration of the Direction 'enum'
    Tetris_DeclareEnum(Direction);

    // Generate the Rotation class
    Tetris_TypedWrapper(Rotation, size_t);

    // Generate the Row class
    Tetris_TypedWrapper(Row, size_t);

    // Generate the Column class
    Tetris_TypedWrapper(Column, size_t);

    // Generate the GameOver class
    Tetris_TypedWrapper(GameOver, bool);


    /**
     * Represents a Tetris block.
     */
    class Block
    {
    public:
        Block(BlockType inType, Rotation inRotation, Row inRow, Column inColumn);

        BlockType type() const;

        // Get the grid associated with this block
        const Grid & grid() const;

        size_t row() const;

        size_t column() const;

        size_t rotation() const;

        size_t numRotations() const;

        void rotate();

        void setRow(size_t inRow);

        void setColumn(size_t inColumn);

        void setRotation(size_t inRotation);

    private:
        BlockType mType;
        size_t mRotation;
        size_t mRow;
        size_t mColumn;
        const Grid * mGrid;
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
