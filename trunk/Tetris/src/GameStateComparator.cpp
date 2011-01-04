#include "Tetris/Config.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Assert.h"


namespace Tetris
{

    GameStateComparator::GameStateComparator()
    {
        throw std::logic_error("Should never come here!");
    }


    GameStateComparator::GameStateComparator(std::auto_ptr<Evaluator> inEvaluator) :
        mEvaluator(inEvaluator.release())
    {
    }


    bool GameStateComparator::operator()(NodePtr lhs, NodePtr rhs)
    {
        Assert(lhs && rhs);
        Assert(lhs.get() != rhs.get());

        // Order by descending quality.
		return lhs->quality() > rhs->quality();
    }

} // namespace Tetris
