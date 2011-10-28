#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Futile/Assert.h"


namespace Tetris {


bool GameStateComparator::operator()(NodePtr lhs, NodePtr rhs) const
{
    // Order by descending quality.
    return lhs->quality() > rhs->quality();
};


} // namespace Tetris
