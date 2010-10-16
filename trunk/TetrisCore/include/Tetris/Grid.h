#ifndef TETRIS_GRID_H_INCLUDED
#define TETRIS_GRID_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/GenericGrid.h"


namespace Tetris
{

    template<class T> class GenericGrid;
    typedef GenericGrid<BlockType> Grid;


} // namespace Tetris



#endif // TETRIS_GRID_H_INCLUDED
