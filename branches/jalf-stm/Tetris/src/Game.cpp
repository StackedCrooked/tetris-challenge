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
#include "Futile/STMSupport.h"
#include "Poco/Exception.h"
#include "Poco/Random.h"
#include <algorithm>
#include <ctime>
#include <stdexcept>


namespace Tetris {


using namespace Futile;
using namespace Futile::STM;
extern const int cMaxLevel;
using std::placeholders::_1;


namespace { // anonymous


Block CreateDefaultBlock(BlockType inBlockType, std::size_t inNumColumns)
{
    return Block(inBlockType,
                 Rotation(0),
                 Row(0),
                 Column(InitialBlockPosition(inNumColumns, GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount())));
}


BlockTypes GenerateBlockTypes(unsigned n)
{
    BlockTypes result;
    BlockFactory blockFactory;
    for (std::size_t idx = 0; idx < cBlockTypeCount * n; ++idx)
    {
        result.push_back(blockFactory.getNext());
    }
    return result;
}


class CircularBlockTypes
{
public:
    CircularBlockTypes(unsigned n) :
        mBlockTypes(GenerateBlockTypes(n))
    {
    }

    BlockType get(std::size_t inIndex) const
    {
        return mBlockTypes[inIndex % mBlockTypes.size()];
    }

    BlockTypes::size_type size() const
    {
        return mBlockTypes.size();
    }

private:
    BlockTypes mBlockTypes;
};


} // anonymous namespace


struct Game::Impl : boost::noncopyable
{
    Impl(Game & inGame,
         unsigned inRowCount,
         unsigned inColumnCount) :
        mGame(inGame),
        mBlockTypes(1),
        mGarbage(1),
        mGarbageIndex(0),
        mActiveBlock(CreateDefaultBlock(mBlockTypes.get(0), inColumnCount)),
        mStartingLevel(0),
        mPaused(false),
        mGameState(GameState(inRowCount, inColumnCount))
    {
    }
    ~Impl(){}

    void commit(const Block & inBlock)
    {
        int lines = stm::atomic<int>([=](stm::transaction & tx) -> int { return this->commit(tx, inBlock); });
        mGame.LinesCleared(lines);
    }

    int commit(stm::transaction & tx, const Block & inBlock)
    {
        GameState & gameState = mGameState.open_rw(tx);
        int lines = gameState.numLines();
        gameState = gameState.commit(inBlock);
        return gameState.numLines() - lines;
    }

    void applyLinePenalty(stm::transaction & tx, std::size_t inLineCount);

    std::vector<BlockType> getGarbageRow(stm::transaction &);

    Game::MoveResult move(stm::transaction & tx, Direction inDirection);

    Game & mGame;
    const CircularBlockTypes mBlockTypes;
    const CircularBlockTypes mGarbage;
    Shared<BlockTypes::size_type> mGarbageIndex;
    Shared<Block> mActiveBlock;
    Shared<int> mStartingLevel;
    Shared<bool> mPaused;
    Shared<GameState> mGameState;
};


Game::Game(std::size_t inNumRows, std::size_t inNumColumns) :
    mImpl(new Impl(*this, inNumRows, inNumColumns))
{
}


Game::~Game()
{
    delete mImpl;
}


void Game::setPaused(bool inPause)
{
    mImpl->mPaused.set(inPause);
}


bool Game::isPaused() const
{
    return mImpl->mPaused.get();
}


unsigned Game::gameStateId() const
{
	// open_rw => eternal loop in 'Computer::Impl::move()' 
    return stm::atomic<unsigned>([=](stm::transaction & tx){ return mImpl->mGameState.open_r(tx).id(); });
}


Block Game::activeBlock() const
{
    return mImpl->mActiveBlock.get();
}


GameStateStats Game::stats() const
{
    return stm::atomic<GameStateStats>([&](stm::transaction & tx) { return mImpl->mGameState.open_r(tx).stats(); });
}


bool Game::isGameOver() const
{
    return stm::atomic<bool>([&](stm::transaction & tx) { return mImpl->mGameState.open_r(tx).isGameOver(); });
}


Grid Game::grid() const
{
    return stm::atomic<Grid>([&](stm::transaction & tx){ return mImpl->mGameState.open_r(tx).grid(); });
}


int Game::rowCount() const
{
    return stm::atomic<int>([&](stm::transaction & tx) { return mImpl->mGameState.open_r(tx).grid().rowCount(); });
}


int Game::columnCount() const
{
    return stm::atomic<int>([&](stm::transaction & tx) { return mImpl->mGameState.open_r(tx).grid().columnCount(); });
}


int Game::level() const
{
    return std::max<int>(stats().numLines() / 10, STM::get(mImpl->mStartingLevel));
}


GameState Game::gameState() const
{
    return mImpl->mGameState.get();
}


std::vector<BlockType> Game::Impl::getGarbageRow(stm::transaction & tx)
{
    auto numCols = mGame.columnCount();
    BlockTypes result(numCols, BlockType_Nil);
    BlockTypes::size_type & garbageIndex = mGarbageIndex.open_rw(tx);

    static Poco::UInt32 fSeed = static_cast<Poco::UInt32>(time(0) % Poco::UInt32(-1));
    fSeed = (fSeed + 1) % Poco::UInt32(-1);
    Poco::Random rand;
    rand.seed(fSeed);

    static const int cMinCount = 4;
    static const int cMaxCount = 8;
    int count = 0;
    while (count < cMinCount)
    {
        for (unsigned idx = 0; idx < unsigned(numCols); ++idx)
        {
            if (result[idx] == BlockType_Nil && rand.nextBool())
            {
                result[idx] = mGarbage.get(garbageIndex++);

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
    stm::atomic([=](stm::transaction & tx) { mImpl->applyLinePenalty(tx, inLineCount); });
}


void Game::Impl::applyLinePenalty(stm::transaction & tx, std::size_t inLineCount)
{
    if (inLineCount <= 1)
    {
        return;
    }

    const GameState & gameState = mGameState.open_r(tx);
    if (gameState.isGameOver())
    {
        return;
    }

    int lineIncrement = inLineCount < 4 ? (inLineCount - 1) : inLineCount;

    int newFirstOccupiedRow = gameState.firstOccupiedRow() - lineIncrement;
    if (newFirstOccupiedRow < 0)
    {
        newFirstOccupiedRow = 0;
    }

    Grid grid = gameState.grid();

    std::size_t garbageStart = grid.rowCount() - lineIncrement;

    std::vector<BlockType> garbageRow;

    for (std::size_t r = newFirstOccupiedRow; r < grid.rowCount(); ++r)
    {
        if (r >= garbageStart)
        {
            garbageRow = getGarbageRow(tx);
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

    mGameState.open_rw(tx).setGrid(grid);

    // Check if the active block has been caught in the penalty lines that were added.
    // If yes then we need to commit the current gamestate.
    const Block & block = mActiveBlock.open_r(tx);
    if (!gameState.checkPositionValid(block, block.row(), block.column()))
    {
        // Commit the game state.
        bool result = move(tx, MoveDirection_Down);
        Assert(!result); // verify commit
        (void)result; // silence compiler warning about unused variable
    }
}


bool Game::checkPositionValid(const Block & inBlock) const
{
    return stm::atomic<bool>([&](stm::transaction & tx) { return mImpl->mGameState.open_r(tx).checkPositionValid(inBlock); });
}


bool Game::canMove(Direction inDirection) const
{
    return stm::atomic<bool>([&](stm::transaction & tx) -> bool {
        const GameState & gs = mImpl->mGameState.open_r(tx);
        if (gs.isGameOver()) {
            return false;
        }

        const Block & block = mImpl->mActiveBlock.open_r(tx);
        std::size_t newRow = block.row()    + GetRowDelta(inDirection);
        std::size_t newCol = block.column() + GetColumnDelta(inDirection);
        return mImpl->mGameState.open_r(tx).checkPositionValid(block, newRow, newCol);
    });
}


Grid Game::gameGrid() const
{
    return stm::atomic<Grid>([this](stm::transaction & tx) { return mImpl->mGameState.open_r(tx).grid(); });
}


BlockTypes Game::getFutureBlocks(std::size_t inCount)
{
    return stm::atomic<BlockTypes>([&](stm::transaction & tx) -> BlockTypes {
        BlockTypes result;

        const auto & blockTypes = mImpl->mBlockTypes; // immutible const
        const GameState & gs = mImpl->mGameState.open_r(tx);
        for (std::size_t idx = 0; idx < inCount; ++idx)
        {
            result.push_back(blockTypes.get(gs.id() + idx));
        }
        Assert(result.size() == inCount);
        return result;
    });
}


Game::MoveResult Game::rotate()
{
    return stm::atomic<Game::MoveResult>([&](stm::transaction & tx) -> Game::MoveResult
    {
        if (isGameOver())
        {
            return MoveResult_NotMoved;
        }

        Block & block = mImpl->mActiveBlock.open_rw(tx);
        std::size_t oldRotation = block.rotation();
        block.rotate();
        const GameState & gameState = mImpl->mGameState.open_r(tx);
        if (!gameState.checkPositionValid(block, block.row(), block.column()))
        {
            block.setRotation(oldRotation);
            return MoveResult_NotMoved;
        }
        return MoveResult_Moved;
    });
}


void Game::dropAndCommit()
{
    while (move(MoveDirection_Down) == MoveResult_Moved);
}


void Game::dropWithoutCommit()
{
    while (canMove(MoveDirection_Down))
    {
        MoveResult result = move(MoveDirection_Down);
        Assert(result != MoveResult_Committed);
        (void)result;
    }
}


void Game::setStartingLevel(int inLevel)
{
    mImpl->mStartingLevel.set(inLevel);
}


Game::MoveResult Game::move(Direction inDirection)
{
    return stm::atomic<Game::MoveResult>([=](stm::transaction & tx) { return mImpl->move(tx, inDirection); });
}


Game::MoveResult Game::Impl::move(stm::transaction & tx, Direction inDirection)
{
    const GameState & cGameState = this->mGameState.open_r(tx);
    if (cGameState.isGameOver())
    {
        return MoveResult_NotMoved;
    }

    const Block & block = mActiveBlock.open_r(tx);
    std::size_t newRow = block.row() + GetRowDelta(inDirection);
    std::size_t newCol = block.column() + GetColumnDelta(inDirection);

    if (cGameState.checkPositionValid(block, newRow, newCol))
    {
        Block & block = mActiveBlock.open_rw(tx);
        block.setRow(newRow);
        block.setColumn(newCol);
        return MoveResult_Moved;
    }

    if (inDirection != MoveDirection_Down)
    {
        // Do nothing
        return MoveResult_NotMoved;
    }

    commit(tx, block);

    // Count the number of lines that were made in the commit call.
    const GameState & newGameState = mGameState.open_r(tx);

    mActiveBlock.set(CreateDefaultBlock(mBlockTypes.get(newGameState.id()), newGameState.grid().columnCount()));

    return MoveResult_Committed;
}


int Game::firstOccupiedRow() const
{
    return stm::atomic<int>([=](stm::transaction & tx) { return mImpl->mGameState.open_rw(tx).firstOccupiedRow(); });
}


} // namespace Tetris
