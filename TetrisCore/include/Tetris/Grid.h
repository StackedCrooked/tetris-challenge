#ifndef TETRIS_GRID_H_INCLUDED
#define TETRIS_GRID_H_INCLUDED


#include "Tetris/Enum.h"


namespace Tetris
{

    template<class T>
    class GenericGrid;

    Tetris_DeclareEnum(BlockType);

    typedef GenericGrid<BlockType> Grid;


} // namespace Tetris



#endif // GRID_H_INCLUDED
