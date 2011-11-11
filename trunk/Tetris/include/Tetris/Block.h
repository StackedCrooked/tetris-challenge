#ifndef TETRIS_BLOCK_H_INCLUDED
#define TETRIS_BLOCK_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Futile/TypedWrapper.h"
#include <memory>


namespace Tetris {



FUTILE_BOX_TYPE(Rotation, unsigned);
FUTILE_BOX_TYPE(Row,      unsigned);
FUTILE_BOX_TYPE(Column,   unsigned);


/**
 * Represents a Tetris block.
 */
class Block
{
public:
    Block(BlockType inType, Rotation inRotation, Row inRow, Column inColumn);

    unsigned identification() const;

    BlockType type() const;

    unsigned rotation() const;

    unsigned rotationCount() const;

    const Grid & grid() const;

    unsigned row() const;

    unsigned column() const;

    unsigned rowCount() const;

    unsigned columnCount() const;

    void rotate();

    void setRow(unsigned inRow);

    void setColumn(unsigned inColumn);

    void setRotation(unsigned inRotation);

private:
    BlockType mType;
    unsigned mRotation;
    unsigned mRow;
    unsigned mColumn;
    const Grid * mGrid;
};


unsigned GetBlockRotationCount(BlockType inType);

// Returns the number possible combinations of rotations and position to
// place a certain block in a grid that has a given a number of columns.
unsigned GetBlockPositionCount(BlockType inType, unsigned inNumColumns);

unsigned GetBlockIdentifier(BlockType inType, unsigned inRotation);

// Gets the Grid object that is associated with a block identifier
const Grid & GetGrid(unsigned inBlockIdentifier);


} // namespace Tetris


#endif // BLOCK_H_INCLUDED
