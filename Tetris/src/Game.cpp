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


namespace Tetris {


using namespace Futile;


extern const int cMaxLevel;


Game::Game(std::size_t inNumRows, std::size_t inNumColumns) :
    mActiveBlock(),
    mBlockFactory(new BlockFactory),
    mGarbageFactory(new BlockFactory),
    mBlocks(),
    mStartingLevel(-1),
    mPaused(false),
    mMuteEvents(false),
    mGameState(inNumRows, inNumColumns)
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


unsigned Game::blockCount() const
{
    return gameState().id();
}


void Game::onChanged()
{
    if (!mMuteEvents)
    {
        GameStateChanged();
    }
}

void Game::onLinesCleared(std::size_t inLineCount)
{
    if (!mMuteEvents)
    {
        LinesCleared(inLineCount);
    }
}


const GameState & Game::gameState() const
{
    return mGameState;
}


GameState & Game::gameState()
{
    return mGameState;
}


void Game::commit(const Block & inBlock)
{
    mGameState.commit(inBlock);
}


std::vector<BlockType> Game::getGarbageRow() const
{
    BlockTypes result(columnCount(), BlockType_Nil);

    static Poco::UInt32 fSeed = static_cast<Poco::UInt32>(time(0) % Poco::UInt32(-1));
    fSeed = (fSeed + 1) % Poco::UInt32(-1);
    Poco::Random rand;
    rand.seed(fSeed);

    static const int cMinCount = 4;
    static const int cMaxCount = 8;
    int count = 0;
    while (count < cMinCount)
    {
        for (unsigned idx = 0; idx < unsigned(columnCount()); ++idx)
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


void Game::applyLinePenalty(std::size_t inLineCount)
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

    std::size_t garbageStart = grid.rowCount() - lineIncrement;

    std::vector<BlockType> garbageRow;

    for (std::size_t r = newFirstOccupiedRow; r < grid.rowCount(); ++r)
    {
        if (r >= garbageStart)
        {
            garbageRow = getGarbageRow();
        }
        for (std::size_t c = 0; c < grid.columnCount(); ++c)
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
        (void)result; // silence compiler warning about unused variable
    }

    onChanged();
}


std::unique_ptr<Block> Game::CreateDefaultBlock(BlockType inBlockType, std::size_t inNumColumns)
{
    return std::unique_ptr<Block>(
        new Block(inBlockType,
                    Rotation(0),
                    Row(0),
                    Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount()))));
}


void Game::supplyBlocks()
{
    while (blockCount() >= mBlocks.size())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


void Game::setPaused(bool inPaused)
{
    LogInfo(SS() << "Game::setPaused: " << inPaused);
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
    return gameGrid().rowCount();
}


int Game::columnCount() const
{
    return gameGrid().columnCount();
}


bool Game::checkPositionValid(const Block & inBlock) const
{
    return gameState().checkPositionValid(inBlock);
}


int Game::GetRowDelta(MoveDirection inDirection)
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


int Game::GetColumnDelta(MoveDirection inDirection)
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


bool Game::canMove(MoveDirection inDirection)
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


void Game::reserveBlocks(std::size_t inCount)
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


void Game::getFutureBlocks(std::size_t inCount, BlockTypes & outBlocks) const
{
    // Make sure we have all blocks we need.
    while (mBlocks.size() < blockCount() + inCount)
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }

    for (std::size_t idx = 0; idx < inCount; ++idx)
    {
        outBlocks.push_back(mBlocks[blockCount() + idx]);
    }
}


void Game::getFutureBlocksWithOffset(std::size_t inOffset, std::size_t inCount, BlockTypes & outBlocks) const
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


bool Game::rotate()
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


void Game::dropAndCommit()
{
    // Local scope for ScopedMute
    {
        ScopedMute scopedMute(mMuteEvents);
        dropWithoutCommit();
        bool result = move(MoveDirection_Down);
        Assert(!result); // check commit
        (void)result; // silence unused variable warning
    }
    onChanged();
}


void Game::dropWithoutCommit()
{
    // Local scope for ScopedMute
    {
        ScopedMute scopedMute(mMuteEvents);
        while (canMove(MoveDirection_Down))
        {
            bool result = move(MoveDirection_Down);
            Assert(result); // no commit
            (void)result; // silence unused variable warning
        }
    }
    onChanged();
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


HumanGame::HumanGame(std::size_t inNumRows, std::size_t inNumCols) :
    Game(inNumRows, inNumCols)
{
}


HumanGame::HumanGame(const Game & inGame) :
    Game(inGame.rowCount(), inGame.columnCount())
{
}


void HumanGame::setGrid(const Grid & inGrid)
{
    gameState().setGrid(inGrid);
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
    int oldLineCount = gameState().numLines();

    // Commit the block. This returns a new GameState object
    // where any full lines have already been cleared.
    commit(block);

    // Count the number of lines that were made in  the commit call.
    int linesCleared = mGameState.numLines() - oldLineCount;
    Assert(linesCleared >= 0);

    // Notify the listeners.
    if (linesCleared > 0)
    {
        onLinesCleared(linesCleared);
    }

    supplyBlocks();
    mActiveBlock.reset(CreateDefaultBlock(mBlocks[blockCount()], gameGrid().columnCount()).release());

    onChanged();
    return false;
}


ComputerGame::ComputerGame(std::size_t inNumRows, std::size_t inNumCols) :
    Game(inNumRows, inNumCols),
    mPrediction()
{
}


ComputerGame::ComputerGame(const Game & inGame) :
    Game(inGame.rowCount(), inGame.columnCount()),
    mPrediction()
{
}


void ComputerGame::setGrid(const Grid & inGrid)
{
    mPrediction.clear();
    gameState().setGrid(inGrid);
    onChanged();
}


void ComputerGame::setCurrentGameState(const GameState & inGameState)
{
    Assert(inGameState->id() == mGameState.id() + 1);

    mGameState = inGameState;

    supplyBlocks();

    mActiveBlock.reset(CreateDefaultBlock(mBlocks[gameState().id()], gameGrid().columnCount()).release());
    onChanged();
}


std::size_t ComputerGame::numPrecalculatedMoves() const
{
    return mPrediction.size();
}


void ComputerGame::clearPrecalculatedNodes()
{
    mPrediction.clear();
}


bool ComputerGame::navigateNodeDown()
{
    if (mPrediction.empty())
    {
        return false;
    }



    const GameState & nextGameState = *mPrediction.begin();
    Assert(nextGameState.id() == gameState().id() + 1);



    int lineDifference = nextGameState.numLines() - gameState().numLines();
    Assert(lineDifference >= 0 && lineDifference <= 4);
    if (lineDifference > 0)
    {
        onLinesCleared(lineDifference);
    }

    setCurrentGameState(nextGameState);
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


    //
    // We can't move the block down any further => we hit the bottom => commit the block
    //

    // Hitting the bottom isn't always a good thing. Especially if the block falls on a place
    // that wasn't planned for. Here we check if the location we're falling to is the location
    // we planned.
    if (!mPrediction.empty())
    {
        const GameState & nextGameState = *mPrediction.begin();
        const Block & nextBlock = nextGameState.originalBlock();

        if (!gameState().tainted())
        {
            // The game is untainted. Good, now check if the blocks line up correctly.
            if (block.column() == nextBlock.column() && nextBlock.identification() == block.identification())
            {
                // Swap the current gamestate with the next precalculated one.
                if (navigateNodeDown())
                {
                    // Return false because the block has not
                    // moved down (it was solidified).
                    return false;
                }
                else
                {
                    LogError("NavigateNodeDown failed for unknown reason (untainted).");
                    Assert(false);
                    mPrediction.clear();
                }
            }
            else
            {
                // The current block has already been solidified and our calculations are no
                // longer valid. Clear all invalid precalcualted nodes.
                LogInfo(SS() << "Too late! Lost nodes: " << mPrediction.size() << " (untained)");
                mPrediction.clear();
            }
        }
        else
        {
            // The game is 'tainted'. Meaning that the gamestate was force changed.
            // Usually this is caused by a multiplayer feature (add penalty lines for example).
            if (block.column() != nextBlock.column() || nextBlock.identification() != block.identification())
            {

                // The precalculated blocks are no longer correct.
                LogInfo(SS() << "Too late! Lost nodes: " << mPrediction.size() << " (tained)");
                mPrediction.clear();
            }
        }
    }

    Assert(mPrediction.empty());
    setCurrentGameState(gameState().commit(block));
    onChanged();
    return false;
}


} // namespace Tetris
