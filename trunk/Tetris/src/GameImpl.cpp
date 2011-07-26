#include "Tetris/Config.h"
#include "Tetris/GameImpl.h"
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


namespace Tetris {


using Futile::InvokeLater;
using Futile::LogError;
using Futile::LogWarning;
using Futile::MakeString;
using Futile::Mutex;
using Futile::Locker;
using Futile::ThreadSafe;


extern const int cMaxLevel;


GameImpl::EventHandler::Instances GameImpl::EventHandler::sInstances;


GameImpl::EventHandler::EventHandler()
{
    sInstances.insert(this);
}


GameImpl::EventHandler::~EventHandler()
{
    sInstances.erase(this);
}


bool GameImpl::EventHandler::Exists(GameImpl::EventHandler * inEventHandler)
{
    return sInstances.find(inEventHandler) != sInstances.end();
}


GameImpl::Instances GameImpl::sInstances;


GameImpl::GameImpl(std::size_t inNumRows, std::size_t inNumColumns) :
    mNumRows(inNumRows),
    mNumColumns(inNumColumns),
    mActiveBlock(),
    mBlockFactory(new BlockFactory),
    mGarbageFactory(new BlockFactory),
    mBlocks(),
    mFutureBlocksCount(3),
    mCurrentBlockIndex(0),
    mStartingLevel(-1),
    mPaused(false),
    mEventHandlers(),
    mMuteEvents(false)
{
    if (mBlocks.empty())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
    mActiveBlock.reset(CreateDefaultBlock(mBlocks.front(), inNumColumns).release());

    sInstances.insert(this);
}


GameImpl::~GameImpl()
{
    sInstances.erase(this);
}


bool GameImpl::Exists(const GameImpl & inGame)
{
    return sInstances.find(&inGame) != sInstances.end();
}


void GameImpl::RegisterEventHandler(ThreadSafe<GameImpl> inGame, EventHandler * inEventHandler)
{
    FUTILE_LOCK(GameImpl & game, inGame)
    {
        if (Exists(game)) // The game may have ended by the time this event arrives.
        {
            game.mEventHandlers.insert(inEventHandler);
        }
    }
}


void GameImpl::UnregisterEventHandler(ThreadSafe<GameImpl> inGame, EventHandler * inEventHandler)
{
    FUTILE_LOCK(GameImpl & game, inGame)
    {
        if (Exists(game)) // The game may have ended by the time this event arrives.
        {
            game.mEventHandlers.erase(inEventHandler);
        }
    }
}


void GameImpl::onChanged()
{
    if (!mMuteEvents)
    {
        // Invoke on main tread
        InvokeLater(boost::bind(&GameImpl::OnChangedImpl, this));
    }
}


bool GameImpl::Exists(GameImpl * inGame)
{
    return sInstances.find(inGame) != sInstances.end();
}


void GameImpl::OnChangedImpl(GameImpl * inGame)
{
    if (!Exists(inGame))
    {
        return;
    }

    EventHandlers::iterator it = inGame->mEventHandlers.begin(), end = inGame->mEventHandlers.end();
    for (; it != end; ++it)
    {
        GameImpl::EventHandler * eventHandler(*it);
        if (!EventHandler::Exists(eventHandler))
        {
            return;
        }

        eventHandler->onGameStateChanged(inGame);
    }
}


void GameImpl::onLinesCleared(int inLineCount)
{
    if (!mMuteEvents)
    {
        // Invoke on main thread
        InvokeLater(boost::bind(&GameImpl::OnLinesClearedImpl, this, inLineCount));
    }
}


void GameImpl::OnLinesClearedImpl(GameImpl * inGame, int inLineCount)
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
        GameImpl::EventHandler * eventHandler(*it);
        if (!EventHandler::Exists(eventHandler))
        {
            LogWarning("OnLinesClearedImpl: This event handler no longer exists.");
            return;
        }
        eventHandler->onLinesCleared(inGame, inLineCount);
    }
}


std::vector<BlockType> GameImpl::getGarbageRow() const
{
    BlockTypes result(mNumColumns, BlockType_Nil);

    static Poco::UInt32 fSeed = static_cast<Poco::UInt32>(time(0) % Poco::UInt32(-1));
    fSeed = (fSeed + 1) % Poco::UInt32(-1);
    Poco::Random rand;
    rand.seed(fSeed);

    static const int cMinCount = 4;
    static const int cMaxCount = 8;
    int count = 0;
    while (count < cMinCount)
    {
        for (std::size_t idx = 0; idx < mNumColumns; ++idx)
        {
            if (result[idx] == BlockType_Nil && rand.nextBool())
            {
                result[idx] = mGarbageFactory->getNext();
                if (++count >= cMaxCount)
                {
                    break;
                }
            }
        }
    }
    return result;
}


void GameImpl::applyLinePenalty(int inLineCount)
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

    // Work with a copy of the current grid.
    Grid grid = gameGrid();

    int garbageStart = grid.rowCount() - lineIncrement;

    std::vector<BlockType> garbageRow;

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

    // Overwrite the grid with our copy.
    setGrid(grid);

    // Check if the active block has been caught in the penalty lines that were added.
    // If yes then we need to commit the current gamestate.
    const Block & block(activeBlock());
    if (!gameState().checkPositionValid(block, block.row(), block.column()))
    {
        // Commit the game state.
        bool result = move(MoveDirection_Down);
        Assert(!result); // verify commit
    }

    onChanged();
}


std::auto_ptr<Block> GameImpl::CreateDefaultBlock(BlockType inBlockType, std::size_t inNumColumns)
{
    return std::auto_ptr<Block>(
        new Block(inBlockType,
                    Rotation(0),
                    Row(0),
                    Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount()))));
}


void GameImpl::supplyBlocks()
{
    while (mCurrentBlockIndex >= mBlocks.size())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


void GameImpl::setPaused(bool inPaused)
{
    LogInfo(MakeString() << "GameImpl::setPaused: " << inPaused);
    mPaused = inPaused;
}


bool GameImpl::isPaused() const
{
    return mPaused;
}


bool GameImpl::isGameOver() const
{
    return gameState().isGameOver();
}


int GameImpl::rowCount() const
{
    return mNumRows;
}


int GameImpl::columnCount() const
{
    return mNumColumns;
}


int GameImpl::GetRowDelta(MoveDirection inDirection)
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


int GameImpl::GetColumnDelta(MoveDirection inDirection)
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


bool GameImpl::canMove(MoveDirection inDirection)
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    std::size_t newRow = block.row()    + GetRowDelta(inDirection);
    std::size_t newCol = block.column() + GetColumnDelta(inDirection);
    return gameState().checkPositionValid(block, newRow, newCol);
}


void GameImpl::reserveBlocks(std::size_t inCount)
{
    while (mBlocks.size() < inCount)
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


const Block & GameImpl::activeBlock() const
{
    Assert(mActiveBlock);
    return *mActiveBlock;
}


const Grid & GameImpl::gameGrid() const
{
    return gameState().grid();
}


void GameImpl::getFutureBlocks(std::size_t inCount, BlockTypes & outBlocks)
{
    // Make sure we have all blocks we need.
    while (mBlocks.size() < mCurrentBlockIndex + inCount)
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }

    for (std::size_t idx = 0; idx < inCount; ++idx)
    {
        outBlocks.push_back(mBlocks[mCurrentBlockIndex + idx]);
    }
}


void GameImpl::getFutureBlocksWithOffset(std::size_t inOffset, std::size_t inCount, BlockTypes & outBlocks)
{
    // Make sure we have all blocks we need.
    while (mBlocks.size() < inOffset + inCount)
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }

    for (std::size_t idx = 0; idx < inCount; ++idx)
    {
        outBlocks.push_back(mBlocks[inOffset + idx]);
    }
}


std::size_t GameImpl::currentBlockIndex() const
{
    return mCurrentBlockIndex;
}


int GameImpl::futureBlocksCount() const
{
    return mFutureBlocksCount;
}


void GameImpl::setFutureBlocksCount(int inFutureBlocksCount)
{
    mFutureBlocksCount = inFutureBlocksCount;
}


bool GameImpl::rotate()
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    std::size_t oldRotation = block.rotation();
    block.rotate();
    if (!gameState().checkPositionValid(block, block.row(), block.column()))
    {
        block.setRotation(oldRotation);
        return false;
    }
    onChanged();
    return true;
}


void GameImpl::dropAndCommit()
{
    // Local scope for ScopedMute
    {
        ScopedMute scopedMute(mMuteEvents);
        dropWithoutCommit();
        bool result = move(MoveDirection_Down);
        Assert(!result); // check commit
    }
    onChanged();
}


void GameImpl::dropWithoutCommit()
{
    // Local scope for ScopedMute
    {
        ScopedMute scopedMute(mMuteEvents);
        while (canMove(MoveDirection_Down))
        {
            bool result = move(MoveDirection_Down);
            Assert(result); // no commit
        }
    }
    onChanged();
}


int GameImpl::level() const
{
    return std::max<int>(gameState().numLines() / 10, mStartingLevel);
}


void GameImpl::setStartingLevel(int inLevel)
{
    mStartingLevel = inLevel;
    onChanged();
}


HumanGame::HumanGame(std::size_t inNumRows, std::size_t inNumCols) :
    GameImpl(inNumRows, inNumCols),
    mGameState(new GameState(inNumRows, inNumCols))
{
}


HumanGame::HumanGame(const GameImpl & inGame) :
    GameImpl(inGame.rowCount(), inGame.columnCount()),
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


bool HumanGame::move(MoveDirection inDirection)
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    std::size_t newRow = block.row() + GetRowDelta(inDirection);
    std::size_t newCol = block.column() + GetColumnDelta(inDirection);
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


ComputerGame::ComputerGame(std::size_t inNumRows, std::size_t inNumCols) :
    GameImpl(inNumRows, inNumCols),
    mCurrentNode(GameStateNode::CreateRootNode(inNumRows, inNumCols).release())
{
}


ComputerGame::ComputerGame(const GameImpl & inGame) :
    GameImpl(inGame.rowCount(), inGame.columnCount()),
    mCurrentNode(new GameStateNode(new GameState(inGame.gameState()), Balanced::Instance()))
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


std::size_t ComputerGame::numPrecalculatedMoves() const
{
    std::size_t countMovesAhead = 0;
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
    Assert(lineDifference >= 0 && lineDifference <= 4);
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

    // Hitting the bottom isn't always a good thing. Especially if the block falls on a place
    // that wasn't planned for. Here we check if the location we're falling to is the location
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
                // Swap the current gamestate with the next precalculated one.
                if (navigateNodeDown())
                {
                    return false;
                }
                else
                {
                    LogError("NavigateNodeDown failed for unknown reason.");
                    mCurrentNode->clearChildren();
                }
            }
            else
            {
                // The current block has already been solidified and our calculations are no
                // longer valid. Clear all invalid precalcualted nodes.
                mCurrentNode->clearChildren();
            }
        }
        else
        {
            // The game is 'tainted'. Meaning that the gamestate was force changed.
            // Usually this is caused by a multiplayer feature (add penalty lines for example).
            // If the game is tainted then all our work is for naught because the precalculated
            // blocks are no longer correct.
            mCurrentNode->clearChildren();
        }
    }


    // Actually commit the block
    NodePtr child(new GameStateNode(mCurrentNode,
                                    mCurrentNode->gameState().commit(block, GameOver(block.row() == 0)).release(),
                                    mCurrentNode->evaluator()));
    mCurrentNode->addChild(child);
    setCurrentNode(child);
    onChanged();
    return false;
}


} // namespace Tetris
