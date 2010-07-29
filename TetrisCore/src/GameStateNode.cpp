#include "GameStateNode.h"
#include "ErrorHandling.h"
#include "GameState.h"
#include "Block.h"

namespace Tetris
{

    GameStateNode::GameStateNode() :
        mParent(0),
        mGameState()
    {

    }


    GameStateNode::GameStateNode(GameStateNode * inParent, GameState * inGameState) :
        mParent(inParent),
        mGameState(inGameState),
        mIsDeadEnd(false)
    {
    }


    const GameState & GameStateNode::state() const
    {
        if (!mGameState)
        {
            throw std::logic_error("Attempt to dereference uninitialized pointer.");
        }
        return *mGameState;
    }


    Children & GameStateNode::children()
    {
        return mChildren;
    }


    const Children & GameStateNode::children() const
    {
        return mChildren;
    }


    GameStateNode * GameStateNode::parent()
    {
        return mParent;
    }


    const GameStateNode * GameStateNode::parent() const
    {
        return mParent;
    }


    bool GameStateNode::isDeadEnd() const
    {
        return mIsDeadEnd;
    }

    void GameStateNode::markAsDeadEnd()
    {
        mIsDeadEnd = true;
    }


    bool ChildPtrCompare::operator()(ChildPtr lhs, ChildPtr rhs)
    {
        LogicAssert(lhs && rhs, "Comparison fails because ChildPtr objects are null!");
        LogicAssert(lhs.get() != rhs.get(), "Comparison game state against itself. This is wrong!");

        // Order by quality descending.
        int q1 = lhs->state().quality();
        int q2 = rhs->state().quality();
        if (q1 != q2)
        {
            // Order by score descending.
            return q2 >= q1;
        }
        else
        {
            // If the scores are equal, then order by pointer.
            return lhs.get() < rhs.get();
        }
    }

} // namespace Tetris
