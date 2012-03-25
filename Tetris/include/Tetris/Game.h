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


    unsigned gameStateId() const;
    unsigned gameStateId(stm::transaction & tx) const;

    // Threaded!
    boost::signals2::signal<void()> GameStateChanged;

    // Threaded!
    boost::signals2::signal<void(unsigned)> LinesCleared;

    void setPaused(stm::transaction & tx, bool inPause);

    bool isPaused(stm::transaction & tx) const;

    bool isGameOver() const;

    bool isGameOver(stm::transaction & tx) const;

    const Block & activeBlock(stm::transaction & tx) const;

    int rowCount(stm::transaction & tx) const;

    int columnCount(stm::transaction & tx) const;

    bool checkPositionValid(stm::transaction & tx, const Block & inBlock) const;

    bool canMove(stm::transaction & tx, Direction inDirection);

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

    int level() const
    {
        return stm::atomic<int>([this](stm::transaction & tx){ return this->level(tx); });
    }

    int level(stm::transaction & tx) const;

    void setStartingLevel(stm::transaction & tx, int inLevel);

    const Grid & gameGrid(stm::transaction & tx) const;

    BlockTypes getFutureBlocks(stm::transaction & tx, std::size_t inCount);

    const GameState & gameState(stm::transaction & tx) const;

    virtual void applyLinePenalty(stm::transaction & tx, std::size_t inLineCount);

private:
    void commit(stm::transaction & tx, const Block & inBlock);
    void setGrid(stm::transaction & tx, const Grid & inGrid);
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
    mutable stm::shared<int> mStartingLevel;
    mutable stm::shared<bool> mPaused;
    mutable stm::shared<GameState> mGameState;
};


} // namespace Tetris


#endif // TETRIS_GAME_H
