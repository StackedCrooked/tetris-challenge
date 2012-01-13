#ifndef TETRIS_GAMESTATECOMPARATOR_H
#define TETRIS_GAMESTATECOMPARATOR_H


#include "Tetris/NodePtr.h"


namespace Tetris {


class GameStateComparator
{
public:
    bool operator()(const NodePtr & lhs, const NodePtr & rhs) const;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATECOMPARATOR_H
