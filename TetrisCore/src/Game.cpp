#include "Game.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"


namespace Tetris
{

    std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
    {
        return std::auto_ptr<Block>(new Block(inBlockType,
                                              Rotation(0), 
                                              Row(0),
                                              Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns()))));
    }


    Game::Game(int inNumRows, int inNumColumns, const BlockTypes & inBlockTypes) :
        mNumRows(inNumRows),
        mNumColumns(inNumColumns),
        mRootNode(GameStateNode::CreateRootNode(inNumRows, inNumColumns).release()),        
        mActiveBlock(),
        mBlockFactory(),
        mBlocks(inBlockTypes),
        mCurrentBlockIndex(0)
    {
        if (mBlocks.empty())
        {
            mBlocks.push_back(mBlockFactory.getNext());        
        }
        mActiveBlock.reset(CreateDefaultBlock(mBlocks.front(), inNumColumns).release());
        mCurrentNode = mRootNode.get();
    }


    GameStateNode * FindCurrentNode(GameStateNode * inRootNode, size_t inDepth)
    {
        if (inDepth == 0)
        {
            return inRootNode;
        }       
        
        if (inRootNode->children().size() == 1)
        {
            return FindCurrentNode(inRootNode->children().begin()->get(), inDepth - 1);
        }
        else if (inRootNode->children().empty())
        {
            throw std::logic_error("There are no nodes at the requested depth at all!");
        }
        else
        {
            throw std::logic_error("Failed to find the current node. Childnodes require cleanup first.");
        }
    }
        
        
    Game::Game(const Game & inGame) :
        mNumRows(inGame.mNumRows),
        mNumColumns(inGame.mNumColumns),
        mRootNode(inGame.mRootNode->clone().release()),
        mCurrentNode(0),
        mActiveBlock(new Block(*(inGame.mActiveBlock))),
        mBlockFactory(inGame.mBlockFactory),
        mBlocks(inGame.mBlocks),
        mCurrentBlockIndex(inGame.mCurrentBlockIndex)
    {
        mCurrentNode = FindCurrentNode(mRootNode.get(), inGame.mCurrentNode->depth());
    }


    std::auto_ptr<Game> Game::clone() const
    {
        return std::auto_ptr<Game>(new Game(*this));
    }

    
    int Game::numRows() const
    {
        return mNumRows;
    }


    int Game::numColumns() const
    {
        return mNumColumns;
    }


    void Game::reserveBlocks(size_t inCount)
    {
        while (mBlocks.size() < inCount)
        {
            mBlocks.push_back(mBlockFactory.getNext());
        }
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


    void Game::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
    {

        // Make sure we have all blocks we need.
        while (mBlocks.size() - mCurrentBlockIndex < inCount)
        {
            mBlocks.push_back(mBlockFactory.getNext());
        }

        
        outBlocks.push_back(mBlocks[mCurrentBlockIndex]);

        if (inCount > 0)
        {
            for (size_t idx = 0; idx < inCount - 1; ++idx)
            {
                outBlocks.push_back(mBlocks[mCurrentBlockIndex + 1 + idx]);
            }
        }
    }


    GameStateNode * Game::currentNode()
    {
        return mCurrentNode;
    }


    const GameStateNode * Game::currentNode() const
    {
        return mCurrentNode;
    }


    void Game::setCurrentNode(GameStateNode * inCurrentNode)
    {
        mCurrentNode = inCurrentNode;
        mCurrentBlockIndex = mCurrentNode->depth();
        supplyBlocks();
        mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());
    }


    bool Game::navigateNodeUp()
    {
        if (mCurrentNode->parent())
        {
            setCurrentNode(mCurrentNode->parent());
            return true;
        }
        return false;
    }


    bool Game::navigateNodeDown()
    {
        if (mCurrentNode->children().empty())
        {
            return false;
        }

        setCurrentNode(mCurrentNode->children().begin()->get());
        return true;
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
                    setCurrentNode((*previous)->get());
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
                        setCurrentNode(it->get());
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
        ChildNodePtr child(new GameStateNode(mCurrentNode, mCurrentNode->state().commit(block, GameOver(block.row() == 0))));
        mCurrentNode->children().insert(child);
        setCurrentNode(child.get());
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

