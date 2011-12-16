#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


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
 * Game is the base class for HumanGame and ComputerGame subclasses.
 *
 * It manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - the root gamestate node
 */
class Game : boost::noncopyable
{
protected:
    /**
     * Constructor is private. Use the factory methods defined in subtype.
     */
    Game(std::size_t inNumRows, std::size_t inNumColumns);

    // Friendship required for destructor.
    friend class Futile::ThreadSafe<Game>;

public:
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

    bool canMove(MoveDirection inDirection);

    virtual bool move(MoveDirection inDirection);

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

protected:
    static int GetRowDelta(MoveDirection inDirection);
    static int GetColumnDelta(MoveDirection inDirection);

    GameState & gameState();

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


class HumanGame : public Game
{
public:
    inline static Futile::ThreadSafe<Game> Create(std::size_t inNumRows, std::size_t inNumColumns)
    {
        return Futile::ThreadSafe<Game>(new HumanGame(inNumRows, inNumColumns));
    }

protected:
    // Friendship required for constructor.
    friend class Game;

    HumanGame(std::size_t inNumRows, std::size_t inNumCols);

    HumanGame(const Game & inGame);
};


class ComputerGame : public Game
{
public:
    inline static Futile::ThreadSafe<Game> Create(std::size_t inNumRows, std::size_t inNumColumns)
    {
        return Futile::ThreadSafe<Game>(new ComputerGame(inNumRows, inNumColumns));
    }

    virtual bool move(MoveDirection inDirection);

    std::size_t numPrecalculatedMoves() const;

    void clearPrecalculatedNodes();

    virtual void setGrid(const Grid & inGrid);

protected:
    // Friendship required for constructor.
    friend class Game;

    ComputerGame(std::size_t inNumRows, std::size_t inNumCols);

    ComputerGame(const Game & inGame);

private:
    bool navigateNodeDown();

    void setCurrentGameState(const GameState & inGameState);

    std::deque<GameState> mPrediction;
};


} // namespace Tetris


#endif // GAME_H_INCLUDED
