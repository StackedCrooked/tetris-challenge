#include "Game.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"


namespace Tetris
{

    static Block & CenterBlock(Block & inNewBlock, size_t inNumColumns)
    {
        size_t column = static_cast<int>(0.5 + (static_cast<float>(inNumColumns - inNewBlock.grid().numColumns())/2));
        inNewBlock.setColumn(column);
        return inNewBlock;
    }


    Game::Game(int inNumRows, int inNumColumns) :
        mRootNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))),
        mBlockFactory(cBlockTypeCount),
        mBlock(CenterBlock(mBlockFactory.getNext(), inNumColumns))
    {
        mCurrentNode = &mRootNode;
    }


    bool Game::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    const Block & Game::activeBlock() const
    {
        return mBlock;
    }


    Block & Game::activeBlock()
    {
        return mBlock;
    }


    GameStateNode & Game::currentNode()
    {
        return *mCurrentNode;
    }


    const GameStateNode & Game::currentNode() const
    {
        return *mCurrentNode;
    }


    bool Game::navigateNodeUp()
    {
        if (mCurrentNode->parent())
        {
            mCurrentNode = mCurrentNode->parent();
            return true;
        }
        return false;
    }


    bool Game::navigateNodeDown()
    {
        if (!mCurrentNode->children().empty())
        {
            mCurrentNode = mCurrentNode->children().begin()->get();
        }
        return false;
    }


    bool Game::navigateNodeLeft()
    {
        if (GameStateNode * parent = mCurrentNode->parent())
        {
            Children & children = parent->children();
            Children::iterator it = children.begin(), end = children.end();
            Children::iterator * previous(0);
            for (; it != end; ++it)
            {
                if (it->get() == mCurrentNode)
                {
                    if (!previous)
                    {
                        return false;
                    }
                    mCurrentNode = (*previous)->get();
                    return true;
                }
                previous = &it;
            }
        }
        return false;
    }


    bool Game::navigateNodeRight()
    {
        if (GameStateNode * parent = mCurrentNode->parent())
        {
            Children & children = parent->children();
            Children::iterator it = children.begin(), end = children.end();
            Children::iterator * previous(0);
            for (; it != end; ++it)
            {
                if (it->get() == mCurrentNode)
                {
                    ++it;
                    if (it != end)
                    {
                        mCurrentNode = it->get();
                        return true;
                    }
                    return false;
                }
            }
        }
        return false;
    }


    static int GetRowDelta(Direction inDirection)
    {
        switch (inDirection)
        {
            case Direction_Up:
            {
                return -1;
            }
            case Direction_Down:
            {
                return 1;
            }
            default:
            {
                return 0;
            }
        }
    }


    static int GetColumnDelta(Direction inDirection)
    {
        switch (inDirection)
        {
            case Direction_Left:
            {
                return -1;
            }
            case Direction_Right:
            {
                return 1;
            }
            default:
            {
                return 0;
            }
        }
    }


    bool Game::move(Direction inDirection)
    {
        if (isGameOver())
        {
            return false;
        }

        const GameState & gameState = mCurrentNode->state();
        size_t newRow = mBlock.row() + GetRowDelta(inDirection);
        size_t newCol = mBlock.column() + GetColumnDelta(inDirection);
        if (gameState.checkPositionValid(mBlock, newRow, newCol))
        {
            mBlock.setRow(newRow);
            mBlock.setColumn(newCol);
            return true;
        }
        
        if (inDirection != Direction_Down)
        {
            // Do nothing
            return false;
        }

        // Commit the block
        ChildPtr child(new GameStateNode(mCurrentNode->state().commit(mBlock, mBlock.row() == 0)));
        mCurrentNode->children().insert(child);
        mCurrentNode = child.get();
    
        // Currently mBlock is a null pointer (due to the copy semantics of std::auto_ptr).
        // Therefore we always (also in the case of game-over) get a new one from the factory.
        mBlock = CenterBlock(mBlockFactory.getNext(), gameState.grid().numColumns());
        return false;
    }


    void Game::rotate()
    {
        if (!isGameOver())
        {
            mBlock.rotate();
        }
    }


    void Game::drop()
    {
        while (move(Direction_Down));
    }


} // namespace Tetris

