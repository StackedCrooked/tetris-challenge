#ifndef BLOCKTYPE_H_INCLUDED
#define BLOCKTYPE_H_INCLUDED


#include "Logger.h"
#include <vector>


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
    enum BlockType
    {
        BlockType_Nil = 0,                  // value 0 enables us to type code like: `if (blockType)' and `if (!blockType)'
        BlockType_Begin,                    // 'begin' is value 1.
        BlockType_I = BlockType_Begin,
        BlockType_J,
        BlockType_L,
        BlockType_O,
        BlockType_S,
        BlockType_T,
        BlockType_Z,
        BlockType_End                       // 'end' is one past the last value (similar to STL iterator behavior)
    };

    static const size_t cBlockTypeCount = static_cast<size_t>(BlockType_End) - 1;

    /**
     * BlockTypes
     *
     * Represents a list of block types.
     */
    typedef std::vector<BlockType> BlockTypes;


    /**
     * ToString
     *
     * Converts a BlockType enum value to string.
     *
     * For logging purposes.
     */
    std::string ToString(const BlockType & inBlockType);


} // namespace Tetris



#endif // BLOCKTYPE_H_INCLUDED
