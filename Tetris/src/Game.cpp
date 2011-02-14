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
#include <ctime>
#include <set>
#include <stdexcept>


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
    mCurrentBlockIndex(0),
    mOverrideLevel(-1),
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
    ScopedReaderAndWriter<Game> rwgame(inGame);
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
    ScopedReaderAndWriter<Game> rwgame(inGame);
    Game * game(rwgame.get());
    if (sInstances.find(game) == sInstances.end())
    {
        LogWarning("Game::UnregisterEventHandler: The game object no longer exists!");
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
    //
    // This code runs in the main thread.
    // No synchronization should be required.
    //

    if (!Exists(inGame))
    {
        LogWarning("Game::OnChangedImpl: The game object no longer exists!");
        return;
    }

    EventHandlers::iterator it = inGame->mEventHandlers.begin(), end = inGame->mEventHandlers.end();
    for (; it != end; ++it)
    {
        Game::EventHandler * eventHandler(*it);
        if (!EventHandler::Exists(eventHandler))
        {
            LogWarning("Game::OnChangedImpl: This event handler no longer exists.");
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
    std::vector<BlockType> result(mNumColumns, BlockType_Nil);

    static Poco::UInt32 fSeed(time(0));
    fSeed = (fSeed + 1) % Poco::UInt32(-1);
    Poco::Random rand;
    rand.seed(fSeed);
    BlockFactory blockFactory(2);

    size_t count = 0;
    while (count < 2)
    {
        for (size_t idx = 0; idx < mNumColumns; ++idx)
        {
            if (rand.nextBool())
            {
                result[idx] = blockFactory.getNext();
                count++;
            }
            if (count >= 8)
            {
                break;
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
    LogInfo(MakeString() << "Line increment: " << lineIncrement);

    int newFirstRow = gameState().firstOccupiedRow() - lineIncrement;
    if (newFirstRow < 0)
    {
        newFirstRow = 0;
    }
    LogInfo(MakeString() << "New first row: " << newFirstRow);

    Grid grid = gameState().grid();

    int garbageStart = grid.rowCount() - lineIncrement;
    LogInfo(MakeString() << "Garbage starts at: " << garbageStart);


    std::vector<BlockType> garbageRow(getGarbageRow());
    int r = std::max<int>(0, newFirstRow);
    for (; r < grid.rowCount(); ++r)
    {
        if (r >= garbageStart)
        {
            garbageRow = getGarbageRow();
        }
        for (int c = 0; c < grid.columnCount(); ++c)
        {
            if (r < garbageStart)
            {
                int ri = r - lineIncrement;
                grid.set(ri, c, grid.get(r, c));
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
    if (gameState().checkPositionValid(inBlock, inBlock.row(), inBlock.column()))
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
    supplyBlocks();
    return *mActiveBlock;
}


const Grid & Game::gameGrid() const
{
    return gameState().grid();
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
    while (move(MoveDirection_Down))
    {
        // Keep going.
    }
}


int Game::level() const
{
    if (mOverrideLevel < 0)
    {
        int level = gameState().numLines() / 10;
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


} // namespace Tetris
