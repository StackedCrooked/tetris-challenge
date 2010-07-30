#include "GameStateNode.h"
#include "ErrorHandling.h"
#include "GameState.h"
#include "Block.h"

namespace Tetris
{

    GameStateNode::GameStateNode(std::auto_ptr<GameState> inGameState) :
        mParent(0),
        mDepth(0),
        mGameState(inGameState.release())
    {

    }


    GameStateNode::GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState) :
        mParent(inParent),
        mDepth(inParent->depth() + 1),
        mGameState(inGameState.release())
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


    int GameStateNode::depth() const
    {
        return mDepth;
    }


    const GameStateNode * GameStateNode::parent() const
    {
        return mParent;
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
