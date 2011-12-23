#ifndef TETRIS_UTILITIES_H
#define TETRIS_UTILITIES_H


#include "Futile/MakeString.h"
#include <stdexcept>


namespace Tetris {


inline unsigned InitialBlockPosition(unsigned gridWidth, unsigned blockWidth)
{
    if (blockWidth >= gridWidth)
    {
        throw std::runtime_error(Futile::SS() << "Grid is to narrow to contain block. Grid width: " << gridWidth << ". Block width: " << blockWidth);
    }
    return unsigned(0.5 + (gridWidth - blockWidth) / 2.0);
}


} // namespace Tetris


#endif // TETRIS_UTILITIES_H
