#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/GenericGrid.h"


namespace Tetris
{

    typedef GenericGrid<BlockType> Grid;

    // Rounded division by 2.
    // To be used for integral values (int, size_t, ...).
    template<class T>
    T DivideByTwo(T inValue)
    {
        return (int)(0.5 + ((float)inValue / (float)2.0));
    }

} // namespace Tetris



#endif // GRID_H_INCLUDED
