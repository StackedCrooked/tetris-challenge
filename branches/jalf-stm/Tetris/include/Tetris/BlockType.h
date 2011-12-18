#ifndef TETRIS_BLOCKTYPE_H
#define TETRIS_BLOCKTYPE_H


#include <stdexcept>


namespace Tetris {


/**
 * BlockType
 *
 * Represents a tetris shape or block type.
 *
 * Use a char because that takes less space
 * which is very significant when building
 * the AI search tree.
 *
 * I = long shape
 * J = J shaped block
 * O = square block
 * etc...
 */
typedef unsigned char BlockType;
enum
{
    BlockType_Nil,
    BlockType_I,
    BlockType_Begin = BlockType_I,
    BlockType_J,
    BlockType_L,
    BlockType_O,
    BlockType_S,
    BlockType_T,
    BlockType_Z,
    BlockType_End
};

static const unsigned cBlockTypeCount = static_cast<unsigned>(BlockType_End) - 1;


inline BlockType increment(BlockType inBlockType)
{
    if (inBlockType == BlockType_Nil)
    {
        throw std::logic_error("Can't rotate the nil block");
    }

    BlockType result = inBlockType + 1;
    if (result == BlockType_End)
    {
        result = BlockType_I;
    }
    return result;
}


} // namespace Tetris


#endif // TETRIS_BLOCKTYPE_H
