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


} // anonymous namespace


Game::CircularBlockTypes::CircularBlockTypes(unsigned n) :
    mBlockTypes(GenerateBlockTypes(n))
{
}


Game::Game(std::size_t inNumRows, std::size_t inNumColumns) :
    mBlockTypes(1),
    mGarbage(1),
    mGarbageIndex(0),
    mActiveBlock(CreateDefaultBlock(mBlockTypes.get(0), inNumColumns)),
    mStartingLevel(50),
    mPaused(false),
    mGameState(GameState(inNumRows, inNumColumns))
{
}


Game::~Game()
{
}


unsigned Game::gameStateId(stm::transaction & tx) const
{
    return mGameState.open_r(tx).id();
}


const GameState & Game::gameState(stm::transaction & tx) const
{
    return mGameState.open_r(tx);
}


void Game::commit(stm::transaction & tx, const Block & inBlock)
{
    GameState & gameState = mGameState.open_rw(tx);
    gameState = gameState.commit(inBlock);
}


std::vector<BlockType> Game::getGarbageRow(stm::transaction & tx)
{
    BlockTypes result(columnCount(tx), BlockType_Nil);
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
        for (unsigned idx = 0; idx < unsigned(columnCount(tx)); ++idx)
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


void Game::applyLinePenalty(stm::transaction & tx, std::size_t inLineCount)
{

    if (inLineCount < 2 || isGameOver(tx))
    {
        return;
    }

    int lineIncrement = inLineCount < 4 ? (inLineCount - 1) : inLineCount;

    int newFirstOccupiedRow = gameState(tx).firstOccupiedRow() - lineIncrement;
    if (newFirstOccupiedRow < 0)
    {
        newFirstOccupiedRow = 0;
    }

    const GameState & gameState = mGameState.open_r(tx);
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
    const Block & block = activeBlock(tx);
    if (!gameState.checkPositionValid(block, block.row(), block.column()))
    {
        // Commit the game state.
        bool result = move(tx, MoveDirection_Down);
        Assert(!result); // verify commit
        (void)result; // silence compiler warning about unused variable
    }
}


void Game::setPaused(stm::transaction & tx, bool inPause)
{
    mPaused.open_rw(tx) = inPause;
}


bool Game::isPaused(stm::transaction & tx) const
{
    return mPaused.open_r(tx);
}


bool Game::isGameOver(stm::transaction & tx) const
{
    return mGameState.open_r(tx).isGameOver();
}


const Block & Game::activeBlock(stm::transaction & tx) const
{
    return mActiveBlock.open_r(tx);
}


int Game::rowCount(stm::transaction & tx) const
{
    return gameGrid(tx).rowCount();
}


int Game::columnCount(stm::transaction & tx) const
{
    return gameGrid(tx).columnCount();
}


bool Game::checkPositionValid(stm::transaction & tx, const Block & inBlock) const
{
    return gameState(tx).checkPositionValid(inBlock);
}


bool Game::canMove(stm::transaction & tx, Direction inDirection)
{
    if (isGameOver(tx))
    {
        return false;
    }

    const Block & block = activeBlock(tx);
    std::size_t newRow = block.row()    + GetRowDelta(inDirection);
    std::size_t newCol = block.column() + GetColumnDelta(inDirection);
    return gameState(tx).checkPositionValid(block, newRow, newCol);
}


const Grid & Game::gameGrid(stm::transaction & tx) const
{
    return gameState(tx).grid();
}


BlockTypes Game::getFutureBlocks(stm::transaction & tx, std::size_t inCount)
{
    BlockTypes result;

    for (std::size_t idx = 0; idx < inCount; ++idx)
    {
        result.push_back(mBlockTypes.get(gameStateId(tx) + idx));
        Assert(result.back() <= 28);
    }
    Assert(result.size() == inCount);
    return result;
}


Game::MoveResult Game::rotate(stm::transaction & tx)
{
    if (isGameOver(tx))
    {
        return MoveResult_NotMoved;
    }

    Block & block = mActiveBlock.open_rw(tx);
    std::size_t oldRotation = block.rotation();
    block.rotate();
    if (!gameState(tx).checkPositionValid(block, block.row(), block.column()))
    {
        block.setRotation(oldRotation);
        return MoveResult_NotMoved;
    }
    return MoveResult_Moved;
}


void Game::dropAndCommit(stm::transaction & tx)
{
    while (move(tx, MoveDirection_Down) == MoveResult_Moved);
}


void Game::dropWithoutCommit(stm::transaction & tx)
{
    while (canMove(tx, MoveDirection_Down))
    {
        MoveResult result = move(tx, MoveDirection_Down);
        Assert(result != MoveResult_Committed);
        (void)result;
    }
}


int Game::level(stm::transaction & tx) const
{
    return std::max<int>(gameState(tx).numLines() / 10, mStartingLevel.open_r(tx));
}


void Game::setStartingLevel(stm::transaction & tx, int inLevel)
{
    mStartingLevel.open_rw(tx) = inLevel;
}


void Game::setGrid(stm::transaction & tx, const Grid & inGrid)
{
    mGameState.open_rw(tx).setGrid(inGrid);
}


Game::MoveResult Game::move(stm::transaction & tx, Direction inDirection)
{
    if (isGameOver(tx))
    {
        return MoveResult_NotMoved;
    }

    const Block & block = activeBlock(tx);
    std::size_t newRow = block.row() + GetRowDelta(inDirection);
    std::size_t newCol = block.column() + GetColumnDelta(inDirection);

    const GameState & cGameState = this->gameState(tx);
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

    // Remember the number of lines in the current game state.
    int oldLineCount = cGameState.numLines();

    commit(tx, block);

    // Count the number of lines that were made in the commit call.
    int linesCleared = cGameState.numLines() - oldLineCount;
    Assert(linesCleared >= 0);
    (void)linesCleared;

    Block & assignActiveBlock = mActiveBlock.open_rw(tx);
    const GameState & newGameState = this->gameState(tx);

    assignActiveBlock = CreateDefaultBlock(mBlockTypes.get(newGameState.id()), newGameState.grid().columnCount());
    return MoveResult_Committed;
}


} // namespace Tetris
