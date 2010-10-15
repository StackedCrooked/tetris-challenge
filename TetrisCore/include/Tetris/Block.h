#ifndef TETRIS_BLOCK_H_INCLUDED
#define TETRIS_BLOCK_H_INCLUDED


#include "Tetris/Enum.h"
#include "Tetris/Grid.h"
#include "Tetris/TypedWrapper.h"


namespace Tetris
{
    
    DeclareEnum(BlockType);
    DeclareEnum(Direction);

    // Generate the Rotation class
    GENERATE_TYPESAFE_WRAPPER(size_t, Rotation)

    // Generate the Row class
    GENERATE_TYPESAFE_WRAPPER(size_t, Row)

    // Generate the Column class
    GENERATE_TYPESAFE_WRAPPER(size_t, Column)

    // Generate the GameOver class
    GENERATE_TYPESAFE_WRAPPER(bool, GameOver)


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
