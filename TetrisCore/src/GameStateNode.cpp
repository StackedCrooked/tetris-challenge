#include "GameStateNode.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"

namespace Tetris
{

    GameStateNode::GameStateNode(std::auto_ptr<GameState> inGameState) :
        mParent(0),
        mDepth(0),
        mGameState(inGameState)
    {

    }


    GameStateNode::GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState) :
        mParent(inParent),
        mDepth(inParent->depth() + 1),
        mGameState(inGameState)
    {
    }


    const GameState & GameStateNode::state() const
    {
        return *mGameState;
    }


    GameState & GameStateNode::state()
    {
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


    void GameStateNode::populate(const Block & inBlock)
    {
        CheckPrecondition(mChildren.empty(), "GameStateNode::populate(): already has children.");
        
        Grid & grid = mGameState->grid();
        for (size_t col = 0; col != grid.numColumns(); ++col)
        {
            Block block = inBlock;
            block.setColumn(0);
            block.setRow(0);
            for (size_t rt = 0; rt != 4; ++rt)
            {
                std::auto_ptr<GameState> newGameState;
                block.setRotation(rt);
                size_t row = 0;
                while (mGameState->checkPositionValid(block, row++, col));
                if (row > 0)
                {
                    block.setRow(row - 1);
                    newGameState = mGameState->commit(block, false);
                }
                else
                {
                    newGameState = mGameState->commit(block, true);
                }
                ChildPtr childState(new GameStateNode(newGameState));
                mChildren.insert(childState);
            }
        }
    }


    bool ChildPtrCompare::operator()(ChildPtr lhs, ChildPtr rhs)
    {
        CheckArgument(lhs && rhs, "Comparison fails because ChildPtr objects are null!");
        CheckArgument(lhs.get() != rhs.get(), "Comparison game state against itself. This is wrong!");

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
