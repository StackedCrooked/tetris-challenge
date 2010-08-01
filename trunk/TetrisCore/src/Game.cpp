#include "Game.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"


namespace Tetris
{

    static Block CenterBlock(const Block & inBlock, size_t inNumColumns)
    {
        Block block = inBlock;
        size_t column = static_cast<int>(0.5 + (static_cast<float>(inNumColumns - inBlock.grid().numColumns())/2));
        block.setColumn(column);
        return block;
    }


    Game::Game(int inNumRows, int inNumColumns) :
        mRootNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))),
        mBlockFactory(cBlockTypeCount),
        mCurrentBlockIndex(0)
    {
        mBlocks.push_back(CenterBlock(mBlockFactory.getNext(), inNumColumns));
        mCurrentNode = &mRootNode;
    }


    bool Game::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    const Block & Game::activeBlock() const
    {
        return mBlocks[mCurrentBlockIndex];
    }


    Block & Game::activeBlock()
    {
        return mBlocks[mCurrentBlockIndex];
    }


    std::vector<Block> Game::getFutureBlocks(size_t inCount) const
    {
        std::vector<Block> blocks;
        blocks.push_back(activeBlock());

        while (mCurrentBlockIndex + inCount - 1 >= mBlocks.size())
        {
            mBlocks.push_back(CenterBlock(mBlockFactory.getNext(), mCurrentNode->state().grid().numColumns()));
        }
        return blocks;
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
        
        Block & block = activeBlock();
        size_t newRow = block.row() + GetRowDelta(inDirection);
        size_t newCol = block.column() + GetColumnDelta(inDirection);
        if (gameState.checkPositionValid(block, newRow, newCol))
        {
            block.setRow(newRow);
            block.setColumn(newCol);
            return true;
        }
        
        if (inDirection != Direction_Down)
        {
            // Do nothing
            return false;
        }

        // Commit the block
        ChildPtr child(new GameStateNode(mCurrentNode->state().commit(block, block.row() == 0)));
        mCurrentNode->children().insert(child);
        mCurrentNode = child.get();
    
        
        mCurrentBlockIndex++;
        while (mCurrentBlockIndex >= mBlocks.size())
        {
            mBlocks.push_back(CenterBlock(mBlockFactory.getNext(), gameState.grid().numColumns()));
        }
        return false;
    }


    void Game::rotate()
    {
        if (!isGameOver())
        {
            activeBlock().rotate();
        }
    }


    void Game::drop()
    {
        while (move(Direction_Down));
    }


} // namespace Tetris

