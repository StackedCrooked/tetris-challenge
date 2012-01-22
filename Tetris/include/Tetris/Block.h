#ifndef TETRIS_BLOCK_H
#define TETRIS_BLOCK_H


#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Futile/TypedWrapper.h"
#include <memory>


namespace Tetris {



FUTILE_BOX_TYPE(Rotation, unsigned);
FUTILE_BOX_TYPE(Row,      unsigned);
FUTILE_BOX_TYPE(Column,   unsigned);


unsigned GetBlockIdentifier(BlockType inType, unsigned inRotation);
unsigned GetBlockRotationCount(BlockType inType);
unsigned GetBlockPositionCount(BlockType inType, unsigned inNumColumns);
const Grid & GetGrid(unsigned inId);


/**
 * Represents a Tetris block.
 */
class Block
{
public:
    Block(BlockType inType, Rotation inRotation, Row inRow, Column inColumn);

    unsigned identification() const { return GetBlockIdentifier(type(), rotation()); }

    BlockType type() const { return mType; }

    unsigned rotation() const { return mRotation; }

    unsigned rotationCount() const { return GetBlockRotationCount(mType); }

    const Grid & grid() const { return *mGrid; }

    unsigned row() const { return mRow; }

    unsigned column() const { return mColumn; }

    unsigned rowCount() const { return mGrid->rowCount(); }

    unsigned columnCount() const { return mGrid->columnCount(); }

    void rotate() { setRotation((mRotation + 1) % GetBlockRotationCount(mType)); }

    void setRow(unsigned inRow) { mRow = inRow; }

    void setColumn(unsigned inColumn) { mColumn = inColumn; }

    void setRotation(unsigned inRotation)
    {
        mRotation = inRotation % GetBlockRotationCount(mType);
        mGrid = &GetGrid(GetBlockIdentifier(mType, mRotation));
    }

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


#endif // TETRIS_BLOCK_H
