#ifndef TETRIS_GRID_H_INCLUDED
#define TETRIS_GRID_H_INCLUDED


#include "Futile/BlockType.h"
#include "Futile/GenericGrid.h"


namespace Futile
{

    typedef GenericGrid<BlockType, Allocator_Malloc> Grid;

} // namespace Futile



#endif // TETRIS_GRID_H_INCLUDED
