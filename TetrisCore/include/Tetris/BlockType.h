#ifndef TETRIS_BLOCKTYPE_H_INCLUDED
#define TETRIS_BLOCKTYPE_H_INCLUDED


namespace Tetris
{

    /**
     * BlockType
     *
     * Represents a tetris shape or block type.
     *
     * I = long shape
     * J = J shaped block
     * O = square block
     * etc...
     */
    typedef char BlockType;
    enum
    {
        BlockType_Nil = 0,
        BlockType_Begin,
        BlockType_I = BlockType_Begin,
        BlockType_J,
        BlockType_L,
        BlockType_O,
        BlockType_S,
        BlockType_T,
        BlockType_Z,
        BlockType_End
    };

    static const size_t cBlockTypeCount = static_cast<size_t>(BlockType_End) - 1;

} // namespace Tetris


#endif // BLOCKTYPE_H_INCLUDED
