#include "GameStateNode.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"

namespace Tetris
{


    std::auto_ptr<GameStateNode> GameStateNode::CreateRootNode(size_t inNumRows, size_t inNumColumns)
    {
        return std::auto_ptr<GameStateNode>(new GameStateNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))));
    }


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


    void CollectAll(GameStateNode * inChild, int inDepth, Children & outChildren)
    {
        if (inDepth > 1)
        {
            Children children = inChild->children();
            for (Children::iterator it = children.begin(); it != children.end(); ++it)
            {
                CollectAll(it->get(), inDepth - 1, outChildren);
            }
        }
        else
        {
            Children children = inChild->children();
            for (Children::iterator it = children.begin(); it != children.end(); ++it)
            {
                outChildren.insert(*it);
            }
        }
    }


    GameStateNode * GameStateNode::bestChild(int inDepth)
    {
        ChildNodePtr result;
        Children collectAll;
        CollectAll(this, inDepth, collectAll);
        if (!collectAll.empty())
        {
            result = *collectAll.begin();
        }
        return result.get();
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
        //CheckPrecondition(mChildren.empty(), "GameStateNode::populate(): already has children.");
        if (!mChildren.empty())
        {
            return;
        }
        
        Grid & grid = mGameState->grid();
        for (size_t col = 0; col != grid.numColumns(); ++col)
        {
            Block block = inBlock;
            block.setColumn(col);
            block.setRow(0);
            for (size_t rt = 0; rt != 4; ++rt)
            {
                std::auto_ptr<GameState> newGameState;
                block.setRotation(rt);
                size_t row = 0;
                while (mGameState->checkPositionValid(block, row, col))
                {
                    block.setRow(row);
                    row++;
                }

                if (row > 0)
                {
                    newGameState = mGameState->commit(block, false);
                }
                else
                {
                    newGameState = mGameState->commit(block, true);
                }
                ChildNodePtr childState(new GameStateNode(this, newGameState));
                mChildren.insert(childState);
            }
        }
    }


    bool ChildPtrCompare::operator()(ChildNodePtr lhs, ChildNodePtr rhs)
    {
        CheckArgument(lhs && rhs, "Comparison fails because ChildNodePtr objects are null!");
        CheckArgument(lhs.get() != rhs.get(), "Comparison game state against itself. This is wrong!");

        return lhs->state().quality() > rhs->state().quality();
    }

} // namespace Tetris
