#ifndef TETRIS_UTILITIES_H_INCLUDED
#define TETRIS_UTILITIES_H_INCLUDED


namespace Tetris
{

    // Rounded division by 2.
    // To be used for integral values (int, size_t, ...).
    template<class T>
    T DivideByTwo(T inValue)
    {
        return (int)(0.5 + ((float)inValue / (float)2.0));
    }

} // namespace Tetris


#endif // UTILITIES_H_INCLUDED
