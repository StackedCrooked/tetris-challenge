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

    unsigned gameStateId() const;

    // Threaded!
    boost::signals2::signal<void()> GameStateChanged;

    // Threaded!
    boost::signals2::signal<void(int)> LinesCleared;

    inline void setPaused(bool inPause)
    {
        stm::atomic([&](stm::transaction & tx){
            setPaused(tx, inPause);
        });
    }

    inline void setPaused(stm::transaction & tx, bool inPause)
    {
        mPaused.open_rw(tx) = inPause;
    }

    inline bool isPaused(stm::transaction & tx) const
    {
        return mPaused.open_r(tx);
    }

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    bool checkPositionValid(const Block & inBlock) const;

    bool canMove(Direction inDirection);

    virtual bool move(Direction inDirection);

    bool rotate();

    void dropWithoutCommit();

    void dropAndCommit();

    int level() const;

    void setStartingLevel(int inLevel);

    const Grid & gameGrid() const;

    BlockTypes getFutureBlocks(std::size_t inCount) const;

    const GameState & gameState() const;

    virtual void applyLinePenalty(std::size_t inLineCount);

private:
    // Friendship required for destructor.
    friend class Futile::ThreadSafe<Game>;

    void commit(const Block & inBlock);

    void setGrid(const Grid & inGrid);

    void onChanged();
    void onLinesCleared(std::size_t inLineCount);

    void reserveBlocks(std::size_t inCount);
    void supplyBlocks();

    std::vector<BlockType> getGarbageRow() const;

    boost::scoped_ptr<BlockFactory> mBlockFactory;
    boost::scoped_ptr<BlockFactory> mGarbageFactory;

public:
    mutable stm::shared<Block> mActiveBlock;
    mutable stm::shared<BlockTypes> mBlockTypes;

private:
    int mStartingLevel;
    mutable stm::shared<bool> mPaused;

    // In order to avoid flooding the queue in certain situations.
    bool mMuteEvents;

    /**
     * Create an instance to make the mMuteEvents false over a certain scope.
     * This is handy when we are running a loop and don't want to trigger for
     * for each event.
     */
    struct ScopedMute : boost::noncopyable
    {
        ScopedMute(bool & value) :
            mValue(value)
        {
            mValue = true;
        }

        ~ScopedMute()
        {
            mValue = false;
        }

        bool & mValue;
    };

    GameState mGameState;
};


} // namespace Tetris


#endif // TETRIS_GAME_H
