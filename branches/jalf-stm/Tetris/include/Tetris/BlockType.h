#ifndef TETRIS_BLOCKTYPE_H_INCLUDED
#define TETRIS_BLOCKTYPE_H_INCLUDED


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
typedef char BlockType;
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


} // namespace Tetris


#endif // BLOCKTYPE_H_INCLUDED
