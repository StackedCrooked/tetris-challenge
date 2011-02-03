#include "Tetris/Config.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Block.h"
#include "Tetris/Utilities.h"
#include "Tetris/Logging.h"
#include "Tetris/Threading.h"
#include "Tetris/Assert.h"
#include "Poco/Exception.h"
#include "Poco/Random.h"
#include <algorithm>
#include <set>
#include <stdexcept>


namespace Tetris {


extern const int cMaxLevel;


HumanGame::HumanGame(size_t inNumRows, size_t inNumCols) :
    Game(inNumRows, inNumCols),
    mGameState(new GameState(inNumRows, inNumCols))
{
}


HumanGame::HumanGame(const Game & inGame) :
    Game(inGame.rowCount(), inGame.columnCount()),
    mGameState(new GameState(inGame.getGameState()))
{
}


GameState & HumanGame::getGameState()
{
    if (!mGameState.get())
    {
        throw std::logic_error("Null pointer deref: mGameState");
    }
    return *mGameState;
}


const GameState & HumanGame::getGameState() const
{
    if (!mGameState.get())
    {
        throw std::logic_error("Null pointer deref: mGameState");
    }
    return *mGameState;
}


void HumanGame::setGrid(const Grid & inGrid)
{
    mGameState->setGrid(inGrid);
    onChanged();
}


static int GetRowDelta(MoveDirection inDirection)
{
    switch (inDirection)
    {
        case MoveDirection_Up:
        {
            return -1;
        }
        case MoveDirection_Down:
        {
            return 1;
        }
        default:
        {
            return 0;
        }
    }
}


static int GetColumnDelta(MoveDirection inDirection)
{
    switch (inDirection)
    {
        case MoveDirection_Left:
        {
            return -1;
        }
        case MoveDirection_Right:
        {
            return 1;
        }
        default:
        {
            return 0;
        }
    }
}


bool HumanGame::move(MoveDirection inDirection)
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    size_t newRow = block.row() + GetRowDelta(inDirection);
    size_t newCol = block.column() + GetColumnDelta(inDirection);
    if (getGameState().checkPositionValid(block, newRow, newCol))
    {
        block.setRow(newRow);
        block.setColumn(newCol);
        onChanged();
        return true;
    }

    if (inDirection != MoveDirection_Down)
    {
        // Do nothing
        return false;
    }

    // Remember the number of lines in the current game state.
    int oldLineCount = mGameState->numLines();

    // Commit the block. This returns a new GameState object
    // where any full lines have already been cleared.
    mGameState.reset(mGameState->commit(block, GameOver(block.row() == 0)).release());

    // Count the number of lines that were made in  the commit call.
    int linesCleared = mGameState->numLines() - oldLineCount;
    Assert(linesCleared >= 0);

    // Notify the listeners.
    if (linesCleared > 0)
    {
        onLinesCleared(linesCleared);
    }

    mCurrentBlockIndex++;
    supplyBlocks();
    mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());

    onChanged();
    return false;
}


ComputerGame::ComputerGame(size_t inNumRows, size_t inNumCols) :
    Game(inNumRows, inNumCols),
    mCurrentNode(GameStateNode::CreateRootNode(inNumRows, inNumCols).release())
{
}


ComputerGame::ComputerGame(const Game & inGame) :
    Game(inGame.rowCount(), inGame.columnCount()),
    mCurrentNode(new GameStateNode(new GameState(inGame.getGameState()), new Balanced))
{
}


GameState & ComputerGame::getGameState()
{
    return const_cast<GameState&>(mCurrentNode->gameState());
}


const GameState & ComputerGame::getGameState() const
{
    return mCurrentNode->gameState();
}


void ComputerGame::setGrid(const Grid & inGrid)
{
    mCurrentNode->setGrid(inGrid);
    onChanged();
}


void ComputerGame::setCurrentNode(NodePtr inCurrentNode)
{
    Assert(inCurrentNode->depth() == mCurrentNode->depth() + 1);

    mCurrentNode = inCurrentNode;
    mCurrentBlockIndex = mCurrentNode->depth();
    supplyBlocks();

    mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());
    onChanged();
}


size_t ComputerGame::numPrecalculatedMoves() const
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


void ComputerGame::clearPrecalculatedNodes()
{
    mCurrentNode->children().clear();
}


const GameStateNode * ComputerGame::currentNode() const
{
    return mCurrentNode.get();
}


const GameStateNode * ComputerGame::lastPrecalculatedNode() const
{
    return mCurrentNode->endNode();
}


void ComputerGame::appendPrecalculatedNode(NodePtr inNode)
{
    mCurrentNode->endNode()->addChild(inNode);
}


bool ComputerGame::navigateNodeDown()
{
    if (mCurrentNode->children().empty())
    {
        return false;
    }

    NodePtr nextNode = *mCurrentNode->children().begin();
    Assert(nextNode->depth() == mCurrentNode->depth() + 1);

    int lineDifference = nextNode->gameState().numLines() - mCurrentNode->gameState().numLines();
    Assert(lineDifference >= 0);
    if (lineDifference > 0)
    {
        onLinesCleared(lineDifference);
    }

    setCurrentNode(nextNode);
    onChanged();
    return true;
}


bool ComputerGame::move(MoveDirection inDirection)
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    size_t newRow = block.row() + GetRowDelta(inDirection);
    size_t newCol = block.column() + GetColumnDelta(inDirection);
    if (mCurrentNode->gameState().checkPositionValid(block, newRow, newCol))
    {
        block.setRow(newRow);
        block.setColumn(newCol);
        onChanged();
        return true;
    }

    if (inDirection != MoveDirection_Down)
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
        const Block & nextBlock = precalculatedChild.gameState().originalBlock();
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
                                    mCurrentNode->gameState().commit(block, GameOver(block.row() == 0)).release(),
                                    new Balanced));
    mCurrentNode->addChild(child);
    setCurrentNode(child);
    onChanged();
    return false;
}



Game::Game(size_t inNumRows, size_t inNumColumns) :
    mNumRows(inNumRows),
    mNumColumns(inNumColumns),
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


Game::~Game()
{
}


void Game::registerEventHandler(EventHandler * inEventHandler)
{
    mEventHandlers.push_back(inEventHandler);
}


void Game::unregisterEventHandler(EventHandler * inEventHandler)
{
    mEventHandlers.erase(
        std::remove(mEventHandlers.begin(),
                    mEventHandlers.end(),
                    inEventHandler));
}


void Game::onChanged()
{
    // Invoke on main tread
    InvokeLater(boost::bind(&Game::onChangedImpl, this));
}


void Game::onChangedImpl()
{
    for (size_t idx = 0; idx < mEventHandlers.size(); ++idx)
    {
        Game::EventHandler * eventHandler(mEventHandlers[idx]);
        eventHandler->onGameStateChanged(this);
    }
}


void Game::onLinesCleared(int inLineCount)
{
    // Invoke on main thread
    InvokeLater(boost::bind(&Game::onLinesClearedImpl, this, inLineCount));
}


void Game::onLinesClearedImpl(int inLineCount)
{
    for (size_t idx = 0; idx < mEventHandlers.size(); ++idx)
    {
        Game::EventHandler * eventHandler(mEventHandlers[idx]);
        eventHandler->onLinesCleared(this, inLineCount);
    }
}


void Game::applyLinePenalty(int inLineCount)
{
    if (inLineCount < 2 || isGameOver())
    {
        return;
    }


    int lineIncrement = inLineCount < 4 ? (inLineCount - 1) : 4;
    int newFirstRow = getGameState().firstOccupiedRow() - lineIncrement;
    if (newFirstRow < 0)
    {
        newFirstRow = 0;
    }
    Grid grid = getGameState().grid();
    int garbageStart = grid.rowCount() - lineIncrement;

    for (int r = newFirstRow + lineIncrement; r < grid.rowCount(); ++r)
    {
        int garbage = 0;
        for (int c = 0; c < grid.columnCount(); ++c)
        {
            if (r < garbageStart)
            {
                grid.set(r - lineIncrement, c, grid.get(r, c));
            }
            else
            {
                // Garbage filler at the bottom.
                static Poco::Random fRandom;
                if (fRandom.nextBool())
                {
                    if (garbage <= grid.columnCount()/2)
                    {
                        static BlockFactory fGarbageFactory;
                        grid.set(r, c, fGarbageFactory.getNext());
                        garbage++;
                    }
                    else
                    {
                        grid.set(r, c, BlockType_Nil);
                    }
                }
                else
                {
                    if (c > grid.columnCount() / 2 && garbage < grid.columnCount() / 2)
                    {
                        static BlockFactory fGarbageFactory;
                        grid.set(r, c, fGarbageFactory.getNext());
                        garbage++;
                    }
                    else
                    {
                        grid.set(r, c, BlockType_Nil);
                    }
                }
            }
        }
    }

    // Set the grid
    getGameState().setGrid(grid);

    // Check if the active block has been caught in the penalty lines that were added.
    // If yes then call drop() to normalize the situation.
    const Block & block(activeBlock());
    if (!getGameState().checkPositionValid(block, block.row(), block.column()))
    {
        drop();
    }
    onChanged();
}


void Game::swapGrid(Game & other)
{
    Grid g = other.gameGrid();
    other.setGrid(gameGrid());
    setGrid(g);
}


void Game::swapActiveBlock(Game & other)
{
    Block b = other.activeBlock();
    other.setActiveBlock(activeBlock());
    setActiveBlock(b);
}


std::auto_ptr<Block> Game::CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
{
    return std::auto_ptr<Block>(
        new Block(inBlockType,
                    Rotation(0),
                    Row(0),
                    Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount()))));
}


void Game::setActiveBlock(const Block & inBlock)
{
    if (getGameState().checkPositionValid(inBlock, inBlock.row(), inBlock.column()))
    {
        mActiveBlock.reset(new Block(inBlock));
        onChanged();
    }
}


void Game::supplyBlocks() const
{
    while (mCurrentBlockIndex >= mBlocks.size())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


bool Game::isGameOver() const
{
    return getGameState().isGameOver();
}


int Game::rowCount() const
{
    return mNumRows;
}


int Game::columnCount() const
{
    return mNumColumns;
}


void Game::reserveBlocks(size_t inCount)
{
    while (mBlocks.size() < inCount)
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


const Block & Game::activeBlock() const
{
    supplyBlocks();
    return *mActiveBlock;
}


const Grid & Game::gameGrid() const
{
    return getGameState().grid();
}


void Game::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
{
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


void Game::getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const
{
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


size_t Game::currentBlockIndex() const
{
    return mCurrentBlockIndex;
}


bool Game::rotate()
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    size_t oldRotation = block.rotation();
    block.rotate();
    if (!getGameState().checkPositionValid(block, block.row(), block.column()))
    {
        block.setRotation(oldRotation);
        return false;
    }
    onChanged();
    return true;
}


void Game::drop()
{
    while (move(MoveDirection_Down))
    {
        // Keep going.
        onChanged();
    }
}


int Game::level() const
{
    if (mOverrideLevel < 0)
    {
        int level = getGameState().numLines() / 10;
        return std::min<int>(level, cMaxLevel);
    }
    else
    {
        return mOverrideLevel;
    }
}


void Game::setLevel(int inLevel)
{
    mOverrideLevel = inLevel;
    onChanged();
}


} // namespace Tetris
