#include "Tetris/Config.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/BlockFactory.h"
#include "Tetris/Block.h"
#include "Tetris/Direction.h"
#include "Tetris/Utilities.h"
#include "Tetris/Logging.h"
#include "Tetris/Assert.h"
#include <algorithm>
#include <set>
#include <boost/scoped_ptr.hpp>


namespace Tetris {


    extern const int cMaxLevel;


    class GameImpl
    {
    public:
        GameImpl(size_t inNumRows, size_t inNumColumns);

        GameImpl(const GameImpl & rhs);

        std::auto_ptr<GameImpl> clone();

        bool isGameOver() const;

        int numRows() const;

        int numColumns() const;
        
        bool move(Direction inDirection);

        bool rotate();

        void drop();

        int level() const;

        void setLevel(int inLevel);

        const Block & activeBlock() const;

        void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

        void getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const;

        size_t currentBlockIndex() const;

        void appendPrecalculatedNode(NodePtr inNode);

        const GameStateNode * currentNode() const;

        const GameStateNode * lastPrecalculatedNode() const;

        bool navigateNodeDown();

        size_t numPrecalculatedMoves() const;

        void clearPrecalculatedNodes();

    private:
        GameImpl & operator=(const GameImpl &);

        void reserveBlocks(size_t inCount);

        static std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns);
        void setCurrentNode(NodePtr inCurrentNode);
        void supplyBlocks() const;

        friend class Game;
        size_t mNumRows;
        size_t mNumColumns;
        NodePtr mCurrentNode;
        boost::scoped_ptr<Block> mActiveBlock;
        boost::scoped_ptr<BlockFactory> mBlockFactory;
        mutable BlockTypes mBlocks;
        size_t mCurrentBlockIndex;
        int mOverrideLevel;
    };


    GameImpl::GameImpl(size_t inNumRows, size_t inNumColumns) :
        mNumRows(inNumRows),
        mNumColumns(inNumColumns),
        mCurrentNode(GameStateNode::CreateRootNode(inNumRows, inNumColumns).release()),
        mActiveBlock(),
        mBlockFactory(new BlockFactory),
        mBlocks(),
        mCurrentBlockIndex(0),
        mOverrideLevel(-1)
    {
        if (mBlocks.empty())
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }
        mActiveBlock.reset(CreateDefaultBlock(mBlocks.front(), inNumColumns).release());
    }


    GameImpl::GameImpl(const GameImpl & rhs) :
        mNumRows(rhs.mNumRows),
        mNumColumns(rhs.mNumColumns),
        mCurrentNode(rhs.mCurrentNode->clone()),
        mActiveBlock(new Block(*rhs.mActiveBlock)),
        mBlockFactory(), // YES! Because we must be CERTAIN that getFutureBlocks() will always return the same result.
                         //      The clone method will provide us with 100 precalculated blocks. After that this clone
                         //      becomes invalid. A runtime_exception will thrown the next time mBlockFactory will be
                         //      accessed.
        mBlocks(rhs.mBlocks),
        mCurrentBlockIndex(rhs.mCurrentBlockIndex),
        mOverrideLevel(-1)
    {
    }


    std::auto_ptr<Block> GameImpl::CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
    {
        return std::auto_ptr<Block>(
            new Block(inBlockType,
                      Rotation(0),
                      Row(0),
                      Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns()))));
    }


    void GameImpl::setCurrentNode(NodePtr inCurrentNode)
    {            
        Assert(inCurrentNode->depth() == mCurrentNode->depth() + 1);

        mCurrentNode = inCurrentNode;
        mCurrentBlockIndex = mCurrentNode->depth();
        supplyBlocks();
        mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());
    }


    void GameImpl::supplyBlocks() const
    {            
        if (!mBlockFactory && (mCurrentBlockIndex >= mBlocks.size()))
        {
            throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
        }

        while (mCurrentBlockIndex >= mBlocks.size())
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }
    }

    
    std::auto_ptr<GameImpl> GameImpl::clone()
    {
        if (!mBlockFactory)
        {
            throw std::runtime_error("You cannot clone a Game that was already cloned.");
        }


        // Make sure we have 100 blocks from the factory.
        // This ensures the AI will have enough to do its
        // precalculation.
        while (mCurrentBlockIndex + 100 > mBlocks.size())
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }
        return Create<GameImpl>(*this);
    }


    int GameImpl::numRows() const
    {
        return mNumRows;
    }


    int GameImpl::numColumns() const
    {
        return mNumColumns;
    }


    void GameImpl::reserveBlocks(size_t inCount)
    {
        if (!mBlockFactory && (mBlocks.size() < inCount))
        {
            throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
        }

        while (mBlocks.size() < inCount)
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }
    }


    bool GameImpl::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    const Block & GameImpl::activeBlock() const
    {
        supplyBlocks();
        return *mActiveBlock;
    }


    void GameImpl::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
    {
        if (!mBlockFactory && (mBlocks.size() < mCurrentBlockIndex + inCount))
        {
            throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
        }

        // Make sure we have all blocks we need.
        while (mBlocks.size() < mCurrentBlockIndex + inCount)
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }

        for (size_t idx = 0; idx < inCount; ++idx)
        {
            outBlocks.push_back(mBlocks[mCurrentBlockIndex + idx]);
        }
    }


    void GameImpl::getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const
    {        
        if (!mBlockFactory && (mBlocks.size() < inOffset + inCount))
        {
            throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
        }


        // Make sure we have all blocks we need.
        while (mBlocks.size() < inOffset + inCount)
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }

        for (size_t idx = 0; idx < inCount; ++idx)
        {
            outBlocks.push_back(mBlocks[inOffset + idx]);
        }
    }


    size_t GameImpl::currentBlockIndex() const
    {
        return mCurrentBlockIndex;
    }


    size_t GameImpl::numPrecalculatedMoves() const
    {
        size_t countMovesAhead = 0;
        const GameStateNode * tmp = mCurrentNode.get();
        while (!tmp->children().empty())
        {
            tmp = tmp->children().begin()->get();
            countMovesAhead++;
        }
        return countMovesAhead;
    }


    void GameImpl::clearPrecalculatedNodes()
    {
        mCurrentNode->children().clear();
    }


    const GameStateNode * GameImpl::currentNode() const
    {
        return mCurrentNode.get();
    }


    const GameStateNode * GameImpl::lastPrecalculatedNode() const
    {
        return mCurrentNode->endNode();
    }


    void GameImpl::appendPrecalculatedNode(NodePtr inNode)
    {
        mCurrentNode->endNode()->addChild(inNode);
    }


    bool GameImpl::navigateNodeDown()
    {
        if (mCurrentNode->children().empty())
        {
            return false;
        }

        NodePtr nextNode = *mCurrentNode->children().begin();
        Assert(nextNode->depth() == mCurrentNode->depth() + 1);
        setCurrentNode(nextNode);
        return true;
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


    bool GameImpl::move(Direction inDirection)
    {
        if (isGameOver())
        {
            return false;
        }

        Block & block = *mActiveBlock;
        size_t newRow = block.row() + GetRowDelta(inDirection);
        size_t newCol = block.column() + GetColumnDelta(inDirection);
        if (mCurrentNode->state().checkPositionValid(block, newRow, newCol))
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


        //
        // We can't move the block down any further => we hit the bottom => commit the block
        //

        // First check if we already have a matching precalculated block.
        if (!mCurrentNode->children().empty())
        {
            const GameStateNode & precalculatedChild = **mCurrentNode->children().begin();
            const Block & nextBlock = precalculatedChild.state().originalBlock();
            Assert(nextBlock.type() == block.type());
            if (block.column() == nextBlock.column() &&
                    block.rotation() == nextBlock.rotation())
            {
                return navigateNodeDown();
            }
        }

        // We don't have a matching precalculating block.
        // => Erase any existing children (should not happen)
        if (!mCurrentNode->children().empty())
        {
            LogWarning("Existing children when commiting a block. They will be deleted.");
            mCurrentNode->children().clear();
        }

        // Actually commit the block
        NodePtr child(new GameStateNode(mCurrentNode,
                                        mCurrentNode->state().commit(block, GameOver(block.row() == 0)),
                                        CreatePoly<Evaluator, Balanced>()));
        mCurrentNode->addChild(child);
        setCurrentNode(child);
        return false;
    }


    bool GameImpl::rotate()
    {
        if (isGameOver())
        {
            return false;
        }

        Block & block = *mActiveBlock;
        size_t oldRotation = block.rotation();
        block.rotate();
        if (!mCurrentNode->state().checkPositionValid(block, block.row(), block.column()))
        {
            block.setRotation(oldRotation);
            return false;
        }
        return true;
    }


    void GameImpl::drop()
    {
        while (move(Direction_Down));
    }


    int GameImpl::level() const 
    {
        if (mOverrideLevel < 0)
        {
            int level = mCurrentNode->state().stats().numLines() / 10;
            return std::min<int>(level, cMaxLevel);
        }
        else
        {
            return mOverrideLevel;
        }
    }


    void GameImpl::setLevel(int inLevel)
    {
        mOverrideLevel = inLevel;
    }



    Game::Game(size_t inNumRows, size_t inNumColumns) :
        mImpl(new GameImpl(inNumRows, inNumColumns))
    {
    }
    
    
    Game::Game(const Game & inGame) :
        mImpl(new GameImpl(*inGame.mImpl))
    {
    }
    
    
    Game::Game(std::auto_ptr<GameImpl> inImpl) :
        mImpl(inImpl.release())
    {
    }


    Game::~Game()
    {
        delete mImpl;
        mImpl = 0;
    }


    std::auto_ptr<Game> Game::clone()
    {
        return std::auto_ptr<Game>(new Game(mImpl->clone()));
    }

    
    bool Game::isGameOver() const
    {
        return mImpl->isGameOver();
    }

    
    int Game::numRows() const
    {
        return mImpl->numRows();
    }


    int Game::numColumns() const
    {
        return mImpl->numColumns();
    }
    

    bool Game::move(Direction inDirection)
    {
        return mImpl->move(inDirection);
    }


    bool Game::rotate()
    {
        return mImpl->rotate();
    }


    void Game::drop()
    {
        mImpl->drop();
    }


    int Game::level() const
    {
        return mImpl->level();
    }


    void Game::setLevel(int inLevel)
    {
        mImpl->setLevel(inLevel);
    }


    const Block & Game::activeBlock() const
    {
        return mImpl->activeBlock();
    }


    void Game::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
    {
        mImpl->getFutureBlocks(inCount, outBlocks);
    }


    void Game::getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const
    {
        mImpl->getFutureBlocksWithOffset(inOffset, inCount, outBlocks);
    }


    size_t Game::currentBlockIndex() const
    {
        return mImpl->currentBlockIndex();
    }


    const GameStateNode * Game::currentNode() const
    {
        return mImpl->currentNode();
    }


    const GameStateNode * Game::lastPrecalculatedNode() const
    {
        return mImpl->lastPrecalculatedNode();
    }


    void Game::appendPrecalculatedNode(NodePtr inNode)
    {
        mImpl->appendPrecalculatedNode(inNode);
    }


    bool Game::navigateNodeDown()
    {
        return mImpl->navigateNodeDown();
    }


    size_t Game::numPrecalculatedMoves() const
    {
        return mImpl->numPrecalculatedMoves();
    }


    void Game::clearPrecalculatedNodes()
    {
        mImpl->clearPrecalculatedNodes();
    }

} // namespace Tetris
