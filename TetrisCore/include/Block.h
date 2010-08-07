#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED


#include "BlockType.h"
#include "Direction.h"
#include "Grid.h"


namespace Tetris
{

    typedef GenericGrid<BlockType> Grid;


#define GENERATE_TYPESAFE_WRAPPER(WrappedType, ClassName) \
    class ClassName \
    { \
    public: \
        explicit ClassName(WrappedType inValue) : mValue(inValue) {} \
        WrappedType get() const \
        { return mValue; } \
    private: \
        WrappedType mValue; \
    };

    // Typesafe wrappers for Rotation, Row and Column.
    // Now you can't accidentally mix up the order of
    // arguments in the Block constructor.
    GENERATE_TYPESAFE_WRAPPER(size_t, Rotation)
    GENERATE_TYPESAFE_WRAPPER(size_t, Row)
    GENERATE_TYPESAFE_WRAPPER(size_t, Column)
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
    
    size_t GetBlockIdentifier(BlockType inType, size_t inRotation);
    
    // Gets the Grid object that is associated with a block identifier
    const Grid & GetGrid(int inBlockIdentifier);
    
    Grid GetIGrid(int rotation);
    Grid GetJGrid(int rotation);
    Grid GetLGrid(int rotation);
    Grid GetOGrid(int rotation);
    Grid GetSGrid(int rotation);
    Grid GetTGrid(int rotation);
    Grid GetZGrid(int rotation);


} // namespace Tetris


#endif // BLOCK_H_INCLUDED
