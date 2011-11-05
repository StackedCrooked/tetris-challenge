#ifndef TETRIS_GAMESTATECOMPARATOR_H
#define TETRIS_GAMESTATECOMPARATOR_H


#include "Tetris/NodePtr.h"


namespace Tetris {


class GameStateComparator
{
public:
    bool operator()(NodePtr lhs, NodePtr rhs) const;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATECOMPARATOR_H
