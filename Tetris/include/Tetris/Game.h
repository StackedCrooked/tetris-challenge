#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include "Futile/STMSupport.h"
#include "stm.hpp"
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <boost/scoped_ptr.hpp>
#include <deque>
#include <functional>
#include <memory>


namespace Tetris {


class Block;
class GameStateNode;
class Game;


/**
 * Game manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - block factory
 *   - paused state
 */
class Game
{
public:
    Game(std::size_t inNumRows, std::size_t inNumColumns);

    ~Game();

    // Threaded!
    boost::signals2::signal<void()> GameStateChanged;

    // Threaded!
    boost::signals2::signal<void(unsigned)> LinesCleared;

    unsigned gameStateId() const;

    void setPaused(bool inPause) { Futile::STM::set(mPaused, inPause); }

    bool isPaused() const { return Futile::STM::get(mPaused); }

    bool isGameOver() const;

    GameStateStats stats() const;

    Grid grid() const;

    Block activeBlock() const { return Futile::STM::get(mActiveBlock); }

    int rowCount() const;

    int columnCount() const;

    int level() const;

    bool checkPositionValid(const Block & inBlock) const;

    bool canMove(Direction inDirection) const;

    enum MoveResult
    {
        MoveResult_Moved,
        MoveResult_NotMoved,
        MoveResult_Committed
    };

    virtual MoveResult move(stm::transaction & tx, Direction inDirection);

    MoveResult rotate();

    void dropWithoutCommit();

    void dropAndCommit();

    int firstOccupiedRow(stm::transaction & tx) const
    { return gameState(tx).firstOccupiedRow(); }

    int firstOccupiedRow() const { return stm::atomic<int>([this](stm::transaction & tx){ return firstOccupiedRow(tx); }); }

    void setStartingLevel(stm::transaction & tx, int inLevel);

    const Grid & gameGrid(stm::transaction & tx) const;

    BlockTypes getFutureBlocks(std::size_t inCount);

    virtual void applyLinePenalty(stm::transaction & tx, std::size_t inLineCount);

    const GameState & gameState(stm::transaction & tx) const;

private:
    void commit(stm::transaction & tx, const Block & inBlock);
    void setGrid(const Grid & inGrid);
    std::vector<BlockType> getGarbageRow(stm::transaction & tx);

    class CircularBlockTypes
    {
    public:
        CircularBlockTypes(unsigned n);

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

    const CircularBlockTypes mBlockTypes;
    const CircularBlockTypes mGarbage;
    stm::shared<BlockTypes::size_type> mGarbageIndex;
    stm::shared<Block> mActiveBlock;
    stm::shared<int> mStartingLevel;
    stm::shared<bool> mPaused;
    stm::shared<GameState> mGameState;
};


} // namespace Tetris


#endif // TETRIS_GAME_H
