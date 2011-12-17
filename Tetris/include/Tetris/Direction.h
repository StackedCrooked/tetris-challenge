#ifndef TETRIS_DIRECTION_H_INCLUDED
#define TETRIS_DIRECTION_H_INCLUDED


namespace Tetris {


enum Direction
{
    MoveDirection_Nil,
    MoveDirection_Up,
    MoveDirection_Down,
    MoveDirection_Left,
    MoveDirection_Right
};


inline int GetRowDelta(Direction inDirection)
{
    switch (inDirection)
    {
        case MoveDirection_Up:   return -1;
        case MoveDirection_Down: return  1;
        default:                 return  0;
    }
}


inline int GetColumnDelta(Direction inDirection)
{
    switch (inDirection)
    {
        case MoveDirection_Left:  return -1;
        case MoveDirection_Right: return  1;
        default:                  return  0;
    }
}


} // namespace Tetris


#endif // DIRECTION_H_INCLUDED
