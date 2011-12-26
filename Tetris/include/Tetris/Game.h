#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include "Futile/Threading.h"
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
class Game : boost::noncopyable
{
public:
    Game(std::size_t inNumRows, std::size_t inNumColumns);

    virtual ~Game();

    unsigned gameStateId(stm::transaction & tx) const;

    // Threaded!
    boost::signals2::signal<void()> GameStateChanged;

    // Threaded!
    boost::signals2::signal<void(int)> LinesCleared;

    inline void setPaused(stm::transaction & tx, bool inPause)
    { mPaused.open_rw(tx) = inPause; }

    inline bool isPaused(stm::transaction & tx) const
    { return mPaused.open_r(tx); }

    inline bool isGameOver(stm::transaction & tx) const
    { return mGameState.open_r(tx).isGameOver(); }

    inline const Block & activeBlock(stm::transaction & tx) const
    { return mActiveBlock.open_r(tx); }

    int rowCount(stm::transaction & tx) const;

    int columnCount(stm::transaction & tx) const;

    bool checkPositionValid(stm::transaction & tx, const Block & inBlock) const;

    bool canMove(stm::transaction & tx, Direction inDirection);

    virtual bool move(stm::transaction & tx, Direction inDirection);

    bool rotate(stm::transaction & tx);

    void dropWithoutCommit(stm::transaction & tx);

    void dropAndCommit(stm::transaction & tx);

    int level(stm::transaction & tx) const;

    inline void setStartingLevel(stm::transaction & tx, int inLevel)
    { mStartingLevel.open_rw(tx) = inLevel; }

    const Grid & gameGrid(stm::transaction & tx) const;

    BlockTypes getFutureBlocks(stm::transaction & tx, std::size_t inCount) const;

    const GameState & gameState(stm::transaction & tx) const;

    virtual void applyLinePenalty(stm::transaction & tx, std::size_t inLineCount);

private:
    // Friendship required for destructor.
    friend class Futile::ThreadSafe<Game>;

    void commit(stm::transaction & tx, const Block & inBlock);

    void setGrid(stm::transaction & tx, const Grid & inGrid);

    void reserveBlocks(stm::transaction & tx, std::size_t inCount);
    void supplyBlocks(stm::transaction & tx);

    std::vector<BlockType> getGarbageRow(stm::transaction & tx) const;

    boost::scoped_ptr<BlockFactory> mBlockFactory;
    boost::scoped_ptr<BlockFactory> mGarbageFactory;

    mutable stm::shared<Block> mActiveBlock;
    mutable stm::shared<BlockTypes> mBlockTypes;
    mutable stm::shared<int> mStartingLevel;
    mutable stm::shared<bool> mPaused;
    mutable stm::shared<GameState> mGameState;
};


} // namespace Tetris


#endif // TETRIS_GAME_H
