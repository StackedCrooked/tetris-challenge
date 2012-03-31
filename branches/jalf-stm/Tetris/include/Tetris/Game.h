#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
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


template<typename T>
struct Property
{
    Property(const T & inValue = T()) :
        mValue(inValue)
    {
    }

    const T & get(stm::transaction & tx) const
    {
        return mValue.open_r(tx);
    }

    T & get(stm::transaction & tx)
    {
        return mValue.open_rw(tx);
    }

    T get() const
    {
        return stm::atomic<T>([&](stm::transaction & tx) { return this->get(tx); });
    }

    void set(stm::transaction & tx, const T & inValue)
    {
        mValue.open_rw(tx) = inValue;
    }

    void set(const T & inValue)
    {
        stm::atomic([&](stm::transaction & tx){ this->set(tx, inValue); });
    }

private:
    mutable stm::shared<T> mValue;
};


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

    GameStateStats stats(stm::transaction & tx) const
    {
        return gameState(tx).stats();
    }

    GameStateStats stats()
    {
        return stm::atomic<GameStateStats>([&](stm::transaction & tx) { return stats(tx); });
    }

    unsigned gameStateId() const;
    unsigned gameStateId(stm::transaction & tx) const;

    // Threaded!
    boost::signals2::signal<void()> GameStateChanged;

    // Threaded!
    boost::signals2::signal<void(unsigned)> LinesCleared;

    const Grid & grid(stm::transaction & tx) const
    {
        return gameState(tx).grid();
    }

    // Return a copy
    Grid grid() const
    {
        return stm::atomic<Grid>([&](stm::transaction & tx){ return grid(tx); });
    }

    void setPaused(bool inPause)
    {
        mPaused.set(inPause);
    }

    bool isPaused() const { return mPaused.get(); }
    bool isPaused(stm::transaction & tx) const { return mPaused.get(tx); }

    bool isGameOver() const;
    bool isGameOver(stm::transaction & tx) const;

    Block activeBlock() const;
    const Block & activeBlock(stm::transaction & tx) const;

    int rowCount() const;
    int rowCount(stm::transaction & tx) const;

    int columnCount() const;
    int columnCount(stm::transaction & tx) const;

    bool checkPositionValid(const Block & inBlock) const;
    bool checkPositionValid(stm::transaction & tx, const Block & inBlock) const;

    bool canMove(Direction inDirection) const;
    bool canMove(stm::transaction & tx, Direction inDirection) const;

    enum MoveResult
    {
        MoveResult_Moved,
        MoveResult_NotMoved,
        MoveResult_Committed
    };

    virtual MoveResult move(stm::transaction & tx, Direction inDirection);

    MoveResult rotate(stm::transaction & tx);

    void dropWithoutCommit(stm::transaction & tx);

    void dropAndCommit(stm::transaction & tx);

    int level(stm::transaction & tx) const;
    int level() const { return stm::atomic<int>([this](stm::transaction & tx){ return this->level(tx); }); }

    int firstOccupiedRow(stm::transaction & tx) const
    { return gameState(tx).firstOccupiedRow(); }

    int firstOccupiedRow() const { return stm::atomic<int>([this](stm::transaction & tx){ return firstOccupiedRow(tx); }); }

    void setStartingLevel(stm::transaction & tx, int inLevel);

    const Grid & gameGrid(stm::transaction & tx) const;

    BlockTypes getFutureBlocks(stm::transaction & tx, std::size_t inCount);

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
    mutable stm::shared<BlockTypes::size_type> mGarbageIndex;
    mutable stm::shared<Block> mActiveBlock;
    Property<int> mStartingLevel;
    Property<bool> mPaused;
    Property<GameState> mGameState;
};


} // namespace Tetris


#endif // TETRIS_GAME_H
