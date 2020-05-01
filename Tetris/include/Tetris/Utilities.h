#ifndef TETRIS_UTILITIES_H_INCLUDED
#define TETRIS_UTILITIES_H_INCLUDED


namespace Tetris {


template<class T>
static T DivideByTwo(T inValue)
{
    return static_cast<int>(0.5 + 0.5* inValue);
}


} // namespace Tetris


#endif // UTILITIES_H_INCLUDED
