#include "Game.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"


namespace Tetris
{

    std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
    {
        return std::auto_ptr<Block>(new Block(
            inBlockType,
            Rotation(0),
            Row(0),
            Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns()))));
    }


    Game::Game(int inNumRows, int inNumColumns) :
        mNumRows(inNumRows),
        mNumColumns(inNumColumns),
        mRootNode(GameStateNode::CreateRootNode(inNumRows, inNumColumns)),
        mBlockFactory(cBlockTypeCount),
        mCurrentBlockIndex(0)
    {
        mBlocks.push_back(mBlockFactory.getNext());
        mActiveBlock = CreateDefaultBlock(mBlocks.front(), inNumColumns);
        mCurrentNode = mRootNode.get();
    }


    bool Game::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    void Game::supplyBlocks() const
    {
        while (mCurrentBlockIndex >= mBlocks.size())
        {
            mBlocks.push_back(mBlockFactory.getNext());
        }
    }


    const Block & Game::activeBlock() const
    {
        supplyBlocks();
        return *mActiveBlock;
    }


    Block & Game::activeBlock()
    {
        supplyBlocks();
        return *mActiveBlock;
    }


    std::vector<BlockType> Game::getFutureBlocks(size_t inCount) const
    {

        // Make sure we have all blocks we need.
        while (mBlocks.size() - mCurrentBlockIndex < inCount)
        {
            mBlocks.push_back(mBlockFactory.getNext());
        }

        std::vector<BlockType> blocks;
        blocks.push_back(mBlocks[mCurrentBlockIndex]);

        if (inCount > 0)
        {
            for (size_t idx = 0; idx < inCount - 1; ++idx)
            {
                blocks.push_back(mBlocks[mCurrentBlockIndex + 1 + idx]);
            }
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


    void Game::setCurrentNode(GameStateNode * inCurrentNode)
    {
        CheckPrecondition(inCurrentNode != 0, "inCurrentNode must not be null.");
        CheckPrecondition(mCurrentBlockIndex == mCurrentNode->depth(), "mCurrentBlockIndex == mCurrentNode->depth() is false.");
        mCurrentNode = inCurrentNode;
        mCurrentBlockIndex = mCurrentNode->depth();
        supplyBlocks();
        mActiveBlock = CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns);
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
            ChildNodes & children = parent->children();
            ChildNodes::iterator it = children.begin(), end = children.end();
            ChildNodes::iterator * previous(0);
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
            ChildNodes & children = parent->children();
            ChildNodes::iterator it = children.begin(), end = children.end();
            ChildNodes::iterator * previous(0);
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
        ChildNodePtr child(new GameStateNode(mCurrentNode, mCurrentNode->state().commit(block, block.row() == 0)));
        mCurrentNode->children().insert(child);
        mCurrentNode = child.get();
            
        mCurrentBlockIndex++;
        supplyBlocks();
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

