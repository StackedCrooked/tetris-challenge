#ifndef BLOCKTYPE_H_INCLUDED
#define BLOCKTYPE_H_INCLUDED


namespace Tetris
{

    enum BlockType
    {
        BlockType_Void = 0,              // value 0 enables us to type code like: `if (blockType)' and `if (!blockType)'
        BlockType_Begin,                 // 'begin' is value 1.
        BlockType_I = BlockType_Begin,
        BlockType_J,
        BlockType_L,
        BlockType_O,
        BlockType_S,
        BlockType_T,
        BlockType_Z,
        BlockType_End                    // 'end' is one past the last value (like stl iterators)
    };

    static const size_t cBlockTypeCount = static_cast<size_t>(BlockType_End) - 1;


} // namespace Tetris



#endif // BLOCKTYPE_H_INCLUDED
