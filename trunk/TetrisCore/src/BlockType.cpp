#include "Tetris/BlockType.h"


namespace Tetris
{

    std::string ToString(const BlockType & inBlockType)
    {
        switch (inBlockType)
        {
            case BlockType_Nil:
            {
                return "Nil";
            }
            case BlockType_I:
            {
                return "I";
            }
            case BlockType_J:
            {
                return "J";
            }
            case BlockType_L:
            {
                return "L";
            }
            case BlockType_O:
            {
                return "O";
            }
            case BlockType_S:
            {
                return "S";
            }
            case BlockType_T:
            {
                return "T";
            }
            case BlockType_Z:
            {
                return "Z";
            }
            default:
            {
                throw std::logic_error(MakeString() << "Invalid enum value for BlockType: " << static_cast<int>(inBlockType));
            }
        }
    }


} // namespace Tetris