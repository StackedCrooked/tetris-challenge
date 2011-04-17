#include "Tetris/Config.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Block.h"
#include "Tetris/Utilities.h"
#include "Futile/Assert.h"
#include "Futile/Logging.h"
#include "Futile/MainThread.h"
#include "Futile/Threading.h"
#include "Poco/Exception.h"
#include "Poco/Random.h"
#include <algorithm>
#include <ctime>
#include <set>
#include <stdexcept>


using Futile::InvokeLater;
using Futile::LogError;
using Futile::LogWarning;
using Futile::MakeString;
using Futile::Mutex;
using Futile::ScopedReader;
using Futile::ScopedWriter;
using Futile::ThreadSafe;


namespace Tetris {


extern const int cMaxLevel;


Game::EventHandler::Instances Game::EventHandler::sInstances;


Game::EventHandler::EventHandler()
{
    sInstances.insert(this);
}


Game::EventHandler::~EventHandler()
{
    sInstances.erase(this);
}


bool Game::EventHandler::Exists(Game::EventHandler * inEventHandler)
{
    return sInstances.find(inEventHandler) != sInstances.end();
}


Game::Instances Game::sInstances;


Game::Game(size_t inNumRows, size_t inNumColumns) :
    mNumRows(inNumRows),
    mNumColumns(inNumColumns),
    mActiveBlock(),
    mBlockFactory(new BlockFactory),
    mBlocks(),
    mFutureBlocksCount(3),
    mCurrentBlockIndex(0),
    mStartingLevel(-1),
    mPaused(false)
{
    if (mBlocks.empty())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
    mActiveBlock.reset(CreateDefaultBlock(mBlocks.front(), inNumColumns).release());

    sInstances.insert(this);
}


Game::~Game()
{
    sInstances.erase(this);
}


void Game::RegisterEventHandler(ThreadSafe<Game> inGame, EventHandler * inEventHandler)
{
    ScopedWriter<Game> rwgame(inGame);
    Game * game(rwgame.get());
    if (sInstances.find(game) == sInstances.end())
    {
        LogWarning("Game::RegisterEventHandler: This game object does not exist!");
        return;
    }

    game->mEventHandlers.insert(inEventHandler);
}


void Game::UnregisterEventHandler(ThreadSafe<Game> inGame, EventHandler * inEventHandler)
{
    ScopedWriter<Game> rwgame(inGame);
    Game * game(rwgame.get());
    if (sInstances.find(game) == sInstances.end())
    {
        return;
    }

    game->mEventHandlers.erase(inEventHandler);
}


void Game::onChanged()
{
    // Invoke on main tread
    InvokeLater(boost::bind(&Game::OnChangedImpl, this));
}


bool Game::Exists(Game * inGame)
{
    return sInstances.find(inGame) != sInstances.end();
}


void Game::OnChangedImpl(Game * inGame)
{
    if (!Exists(inGame))
    {
        return;
    }

    EventHandlers::iterator it = inGame->mEventHandlers.begin(), end = inGame->mEventHandlers.end();
    for (; it != end; ++it)
    {
        Game::EventHandler * eventHandler(*it);
        if (!EventHandler::Exists(eventHandler))
        {
            return;
        }

        eventHandler->onGameStateChanged(inGame);
    }
}


void Game::onLinesCleared(int inLineCount)
{
    // Invoke on main thread
    InvokeLater(boost::bind(&Game::OnLinesClearedImpl, this, inLineCount));
}


void Game::OnLinesClearedImpl(Game * inGame, int inLineCount)
{
    //
    // This code runs in the main thread.
    // No synchronization should be required.
    //

    if (!Exists(inGame))
    {
        LogWarning("Game::OnLinesCleared: The game object no longer exists!");
        return;
    }

    EventHandlers::iterator it = inGame->mEventHandlers.begin(), end = inGame->mEventHandlers.end();
    for (; it != end; ++it)
    {
        Game::EventHandler * eventHandler(*it);
        if (!EventHandler::Exists(eventHandler))
        {
            LogWarning("OnLinesClearedImpl: This event handler no longer exists.");
            return;
        }
        eventHandler->onLinesCleared(inGame, inLineCount);
    }
}


std::vector<BlockType> Game::getGarbageRow() const
{
    BlockTypes result(mNumColumns, BlockType_Nil);

    static Poco::UInt32 fSeed = static_cast<Poco::UInt32>(time(0) % Poco::UInt32(-1));
    fSeed = (fSeed + 1) % Poco::UInt32(-1);
    Poco::Random rand;
    rand.seed(fSeed);
    BlockFactory blockFactory(1);

    static const int cMinCount = 4;
    static const int cMaxCount = 8;
    int count = 0;
    while (count < cMinCount)
    {
        for (size_t idx = 0; idx < mNumColumns; ++idx)
        {
            if (result[idx] == BlockType_Nil && rand.nextBool())
            {
                result[idx] = blockFactory.getNext();
                if (++count >= cMaxCount)
                {
                    break;
                }
            }
        }
    }
    return result;
}


void Game::applyLinePenalty(int inLineCount)
{
    if (inLineCount < 2 || isGameOver())
    {
        return;
    }

    int lineIncrement = inLineCount < 4 ? (inLineCount - 1) : inLineCount;

    int newFirstOccupiedRow = gameState().firstOccupiedRow() - lineIncrement;
    if (newFirstOccupiedRow < 0)
    {
        newFirstOccupiedRow = 0;
    }

    Grid grid = gameGrid();
    int garbageStart = grid.rowCount() - lineIncrement;

    std::vector<BlockType> garbageRow(getGarbageRow());

    for (int r = newFirstOccupiedRow; r < grid.rowCount(); ++r)
    {
        if (r >= garbageStart)
        {
            garbageRow = getGarbageRow();
        }
        for (int c = 0; c < grid.columnCount(); ++c)
        {
            if (r < garbageStart)
            {
                grid.set(r, c, grid.get(r + lineIncrement, c));
            }
            else
            {
                grid.set(r, c, garbageRow[c]);
            }
        }
    }

    // Set the grid
    setGrid(grid);

    // Check if the active block has been caught in the penalty lines that were added.
    // If yes then call drop() to normalize the situation.
    const Block & block(activeBlock());
    if (!gameState().checkPositionValid(block, block.row(), block.column()))
    {
        drop();
    }
    onChanged();
}


//void Game::swapGrid(Game & other)
//{
//    Grid g = other.gameGrid();
//    other.setGrid(gameGrid());
//    setGrid(g);
//}


//void Game::swapActiveBlock(Game & other)
//{
//    Block b = other.activeBlock();
//    other.setActiveBlock(activeBlock());
//    setActiveBlock(b);
//}


//void Game::setActiveBlock(const Block & inBlock)
//{
//    if (gameState().checkPositionValid(inBlock, inBlock.row(), inBlock.column()))
//    {
//        mActiveBlock.reset(new Block(inBlock));
//        onChanged();
//    }
//}


std::auto_ptr<Block> Game::CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
{
    return std::auto_ptr<Block>(
        new Block(inBlockType,
                    Rotation(0),
                    Row(0),
                    Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount()))));
}


void Game::supplyBlocks()
{
    while (mCurrentBlockIndex >= mBlocks.size())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


void Game::setPaused(bool inPaused)
{
    LogInfo(MakeString() << "Game::setPaused: " << inPaused);
    mPaused = inPaused;
}


bool Game::isPaused() const
{
    return mPaused;
}


bool Game::isGameOver() const
{
    return gameState().isGameOver();
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
    Assert(mActiveBlock);
    return *mActiveBlock;
}


const Grid & Game::gameGrid() const
{
    return gameState().grid();
}


void Game::getFutureBlocks(size_t inCount, BlockTypes & outBlocks)
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


void Game::getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks)
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


int Game::futureBlocksCount() const
{
    return mFutureBlocksCount;
}


void Game::setFutureBlocksCount(int inFutureBlocksCount)
{
    mFutureBlocksCount = inFutureBlocksCount;
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
    if (!gameState().checkPositionValid(block, block.row(), block.column()))
    {
        block.setRotation(oldRotation);
        return false;
    }
    onChanged();
    return true;
}


void Game::drop()
{
    if (isGameOver())
    {
        return;
    }
    while(move(MoveDirection_Down));
}


int Game::level() const
{
    return std::max<int>(gameState().numLines() / 10, mStartingLevel);
}


void Game::setStartingLevel(int inLevel)
{
    mStartingLevel = inLevel;
    onChanged();
}


HumanGame::HumanGame(size_t inNumRows, size_t inNumCols) :
    Game(inNumRows, inNumCols),
    mGameState(new GameState(inNumRows, inNumCols))
{
}


HumanGame::HumanGame(const Game & inGame) :
    Game(inGame.rowCount(), inGame.columnCount()),
    mGameState(new GameState(inGame.gameState()))
{
}


GameState & HumanGame::gameState()
{
    if (!mGameState.get())
    {
        throw std::logic_error("Null pointer deref: mGameState");
    }
    return *mGameState;
}


const GameState & HumanGame::gameState() const
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
    if (gameState().checkPositionValid(block, newRow, newCol))
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
    mCurrentNode(new GameStateNode(new GameState(inGame.gameState()), new Balanced))
{
}


GameState & ComputerGame::gameState()
{
    return const_cast<GameState&>(mCurrentNode->gameState());
}


const GameState & ComputerGame::gameState() const
{
    return mCurrentNode->gameState();
}


void ComputerGame::setGrid(const Grid & inGrid)
{
    mCurrentNode->clearChildren();
    gameState().setGrid(inGrid);
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
    mCurrentNode->clearChildren();
}


const GameStateNode * ComputerGame::currentNode() const
{
    return mCurrentNode.get();
}


const GameStateNode * ComputerGame::endNode() const
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

    // Hitting the bottom isn't always a good thing. Especially if you fall in a place
    // you didn't plan for. Here we check if the location we're falling to is the location
    // we planned.
    if (!mCurrentNode->children().empty())
    {
        const GameStateNode & precalculatedChild = **mCurrentNode->children().begin();
        const Block & nextBlock = precalculatedChild.gameState().originalBlock();


        if (!currentNode()->gameState().tainted())
        {
            // The game is untainted. Good, now check if the blocks line up correctly.
            if (block.column() == nextBlock.column() && nextBlock.identification() == block.identification())
            {
                // We don't actually commit the block. Instead we swap the current gamestate with
                // the next precalculated one.
                return navigateNodeDown();
            }
            else
            {
                LogError("This current game state has been dirtied by unknown forces.");
                mCurrentNode->clearChildren();
            }
        }
        else
        {
            // The game is 'tainted'. Meaning that the gamestate was force changed by
            // an outsider force. Usually this is some multiplayer penalty or something.
            // Anyway, it means that all our work is for naught because our precalculated
            // blocks are no longer correct.
            mCurrentNode->clearChildren();
        }
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


} // namespace Tetris
