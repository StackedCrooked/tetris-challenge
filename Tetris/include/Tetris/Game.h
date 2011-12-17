#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include "Futile/Threading.h"
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <boost/scoped_ptr.hpp>
#include <deque>
#include <memory>
#include <set>


namespace Tetris {


class Block;
class GameStateNode;
class Game;


/**
 * Game manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - the root gamestate node
 */
class Game : boost::noncopyable
{
public:
    Game(std::size_t inNumRows, std::size_t inNumColumns);

    virtual ~Game();

    unsigned blockCount() const;

    /// GameStateChanged signals change events.
    /// NOTE: callbacks can be received from different threads.
    boost::signals2::signal<void()> GameStateChanged;

    /// LinesCleared signals when clearing lines.
    /// NOTE: callbacks can be received from different threads.
    boost::signals2::signal<void(int)> LinesCleared;

    void setPaused(bool inPause);

    bool isPaused() const;

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

    const Block & activeBlock() const;

    const Grid & gameGrid() const;

    void getFutureBlocks(std::size_t inCount, BlockTypes & outBlocks) const;

    void getFutureBlocksWithOffset(std::size_t inOffset, std::size_t inCount, BlockTypes & outBlocks) const;

    const GameState & gameState() const;

    virtual void applyLinePenalty(std::size_t inLineCount);

    virtual void setGrid(const Grid & inGrid);

private:
    // Friendship required for destructor.
    friend class Futile::ThreadSafe<Game>;

    void commit(const Block & inBlock);

    void onChanged();
    void onLinesCleared(std::size_t inLineCount);

    static std::unique_ptr<Block> CreateDefaultBlock(BlockType inBlockType, std::size_t inNumColumns);
    void reserveBlocks(std::size_t inCount);
    void supplyBlocks();

    std::vector<BlockType> getGarbageRow() const;

    boost::scoped_ptr<Block> mActiveBlock;
    boost::scoped_ptr<BlockFactory> mBlockFactory;
    boost::scoped_ptr<BlockFactory> mGarbageFactory;
    mutable BlockTypes mBlocks;
    int mStartingLevel;
    bool mPaused;

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
