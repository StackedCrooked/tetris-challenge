#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Assert.h"


namespace Tetris
{

    GameStateComparisonFunctor::GameStateComparisonFunctor()
    {
        throw std::logic_error("Should never come here!");
    }


    GameStateComparisonFunctor::GameStateComparisonFunctor(std::auto_ptr<Evaluator> inEvaluator) :
        mEvaluator(inEvaluator.release())
    {
    }


    bool GameStateComparisonFunctor::operator()(NodePtr lhs, NodePtr rhs)
    {
        Assert(lhs && rhs);
        Assert(lhs.get() != rhs.get());

        // Order by descending quality.
        return lhs->state().quality(*mEvaluator) > rhs->state().quality(*mEvaluator);
    }

} // namespace Tetris
