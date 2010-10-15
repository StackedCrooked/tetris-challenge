#ifndef TETRIS_DIRECTION_H_INCLUDED
#define TETRIS_DIRECTION_H_INCLUDED


#include "Tetris/Tetris.h"
#include "Tetris/Enum.h"


namespace Tetris
{

    Tetris_DefineEnum(Direction)
    {
        Direction_Nil,
        Direction_Up,
        Direction_Down,
        Direction_Left,
        Direction_Right
    };

} // namespace Tetris


#endif // DIRECTION_H_INCLUDED
