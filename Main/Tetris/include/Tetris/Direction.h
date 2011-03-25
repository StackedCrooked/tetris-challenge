#ifndef TETRIS_DIRECTION_H_INCLUDED
#define TETRIS_DIRECTION_H_INCLUDED


namespace Tetris {


enum MoveDirection
{
    MoveDirection_Nil,
    MoveDirection_Up,
    MoveDirection_Down,
    MoveDirection_Left,
    MoveDirection_Right
};


// TODO: use this.
enum RotationDirection
{
    RotationDirection_Left,
    RotationDirection_Right
};


} // namespace Tetris


#endif // DIRECTION_H_INCLUDED
