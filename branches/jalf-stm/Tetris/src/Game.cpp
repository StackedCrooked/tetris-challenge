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
#include <stdexcept>


namespace Tetris {


using namespace Futile;
extern const int cMaxLevel;


namespace { // anonymous


Block CreateDefaultBlock(BlockType inBlockType, std::size_t inNumColumns)
{
    return Block(inBlockType,
                 Rotation(0),
                 Row(0),
                 Column(InitialBlockPosition(inNumColumns, GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount())));
}


} // anonymous namespace


Game::Game(std::size_t inNumRows, std::size_t inNumColumns) :
    mBlockFactory(new BlockFactory),
    mGarbageFactory(new BlockFactory),
    mActiveBlock(CreateDefaultBlock(mBlockFactory->getNext(), inNumColumns)),
    mBlockTypes(BlockTypes()),
    mStartingLevel(-1),
    mPaused(false),
    mMuteEvents(false),
    mGameState(inNumRows, inNumColumns)
{
    stm::atomic([&](stm::transaction & tx) {
        mBlockTypes.open_rw(tx).push_back(mActiveBlock.open_r(tx).type());
    });
}


Game::~Game()
{
    mBlockFactory.reset();
    mGarbageFactory.reset();
}


unsigned Game::gameStateId() const
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


void Game::commit(const Block & inBlock)
{
    mGameState = mGameState.commit(inBlock);
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
    stm::atomic([&](stm::transaction & tx) {

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
        Grid grid = mGameState.grid();

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
        const Block & block = mActiveBlock.open_r(tx);
        if (!gameState().checkPositionValid(block, block.row(), block.column()))
        {
            // Commit the game state.
            bool result = move(MoveDirection_Down);
            Assert(!result); // verify commit
            (void)result; // silence compiler warning about unused variable
        }
    });
    onChanged();
}


void Game::supplyBlocks()
{
    stm::atomic([&](stm::transaction & tx) {
        const BlockTypes & cBlocks = mBlockTypes.open_r(tx);
        if (cBlocks.size() > gameStateId())
        {
            return;
        }

        BlockTypes & blocks = mBlockTypes.open_rw(tx);
        while (blocks.size() <= gameStateId())
        {
            blocks.push_back(mBlockFactory->getNext());
        }
    });
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


bool Game::canMove(Direction inDirection)
{
    if (isGameOver())
    {
        return false;
    }

    return stm::atomic<bool>([&](stm::transaction & tx){
        const Block & block = mActiveBlock.open_r(tx);
        std::size_t newRow = block.row()    + GetRowDelta(inDirection);
        std::size_t newCol = block.column() + GetColumnDelta(inDirection);
        return gameState().checkPositionValid(block, newRow, newCol);
    });
}


void Game::reserveBlocks(std::size_t inCount)
{
    stm::atomic([&](stm::transaction & tx) {
        const BlockTypes & cBlocks = mBlockTypes.open_r(tx);
        if (cBlocks.size() >= inCount)
        {
            return;
        }

        BlockTypes & blocks = mBlockTypes.open_rw(tx);
        while (blocks.size() <= inCount)
        {
            blocks.push_back(mBlockFactory->getNext());
        }
    });
}


const Grid & Game::gameGrid() const
{
    return gameState().grid();
}


BlockTypes Game::getFutureBlocks(std::size_t inCount) const
{
    // Make sure we have all blocks we need.
    return stm::atomic<BlockTypes>([&](stm::transaction & tx) {
        const BlockTypes & cBlockTypes = mBlockTypes.open_r(tx);
        if (cBlockTypes.size() >= gameStateId() + inCount)
        {
            BlockTypes result;
            for (std::size_t idx = 0; idx < inCount; ++idx)
            {
                result.push_back(cBlockTypes[gameStateId() + idx]);
                Assert(result.back() <= 28);
            }
            return result;
        }

        BlockTypes & blockTypes = mBlockTypes.open_rw(tx);
        while (blockTypes.size() < gameStateId() + inCount)
        {
            blockTypes.push_back(mBlockFactory->getNext());
            Assert(blockTypes.back() <= 28);
        }
        Assert(blockTypes.size() == gameStateId() + inCount);

        BlockTypes result;
        for (std::size_t idx = 0; idx < inCount; ++idx)
        {
            result.push_back(blockTypes[gameStateId() + idx]);
            Assert(result.back() <= 28);
        }
        Assert(result.size() == inCount);
        return result;
    });
}


//void Game::getFutureBlocksWithOffset(std::size_t inOffset, std::size_t inCount, BlockTypes & outBlocks) const
//{
//    // Make sure we have all blocks we need.
//    while (mBlocks.size() < gameStateId() + inOffset + inCount)
//    {
//        mBlocks.push_back(mBlockFactory->getNext());
//    }

//    for (std::size_t idx = 0; idx < inCount; ++idx)
//    {
//        outBlocks.push_back(mBlocks[inOffset + idx]);
//    }
//}


bool Game::rotate()
{
    if (isGameOver())
    {
        return false;
    }

    bool result = stm::atomic<bool>([&](stm::transaction & tx) {
        Block & block = mActiveBlock.open_rw(tx);
        std::size_t oldRotation = block.rotation();
        block.rotate();
        if (!gameState().checkPositionValid(block, block.row(), block.column()))
        {
            block.setRotation(oldRotation);
            return false;
        }
        return true;
    });

    if (result) {
        onChanged();
    }
    return result;
}


void Game::dropAndCommit()
{
    // Local scope for ScopedMute
    {
        ScopedMute scopedMute(mMuteEvents);
        while (move(MoveDirection_Down));
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


void Game::setGrid(const Grid & inGrid)
{
    mGameState.setGrid(inGrid);
    onChanged();
}


bool Game::move(Direction inDirection)
{
    if (isGameOver())
    {
        return false;
    }

    bool result = stm::atomic<bool>([&](stm::transaction & tx)
    {
        const Block & block = mActiveBlock.open_r(tx);
        std::size_t newRow = block.row() + GetRowDelta(inDirection);
        std::size_t newCol = block.column() + GetColumnDelta(inDirection);

        if (gameState().checkPositionValid(block, newRow, newCol))
        {
            Block & block = mActiveBlock.open_rw(tx);
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

        commit(block);

        // Count the number of lines that were made in the commit call.
        int linesCleared = mGameState.numLines() - oldLineCount;
        Assert(linesCleared >= 0);

        // Notify the listeners.
        if (linesCleared > 0)
        {
            onLinesCleared(linesCleared);
        }

        supplyBlocks();
        mActiveBlock.open_rw(tx) = CreateDefaultBlock(mBlockTypes.open_r(tx)[gameStateId()], gameGrid().columnCount());
        return false;
    });
    onChanged();
    return result;
}


} // namespace Tetris
