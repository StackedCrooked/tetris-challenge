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
#include <boost/scoped_ptr.hpp>
#include <memory>
#include <set>




namespace Tetris {


class Block;
class GameStateNode;
class GameImpl;
using Futile::ThreadSafe;


/**
 * GameImpl is the base class for HumanGame and ComputerGame subclasses.
 *
 * It manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - the root gamestate node
 */
class GameImpl
{
protected:
    /**
     * Constructor is private. Use the factory methods defined in subtype.
     */
    GameImpl(std::size_t inNumRows, std::size_t inNumColumns);

    // Friendship required for destructor.
    friend class ThreadSafe<GameImpl>;

public:
    virtual ~GameImpl();

    class EventHandler
    {
    public:
        EventHandler();

        virtual ~EventHandler();

        // Check if the give EventHandler object still exists
        static bool Exists(EventHandler * inEventHandler);

        // Notifies that the game state has changed.
        // This method arrives in the main thread.
        //
        // The GameImpl* pointer is unlocked at this moment. The
        // user is responsible for locking the corresponding
        // ThreadSafe<Game> object!
        virtual void onGameStateChanged(GameImpl * inGame) = 0;

        virtual void onLinesCleared(GameImpl * inGame, int inLineCount) = 0;

    private:
        EventHandler(const EventHandler&);
        EventHandler& operator=(const EventHandler&);

        typedef std::set<EventHandler*> Instances;
        static Instances sInstances;
    };

    static void RegisterEventHandler(ThreadSafe<GameImpl> inGame, EventHandler * inEventHandler);

    static void UnregisterEventHandler(ThreadSafe<GameImpl> inGame, EventHandler * inEventHandler);

    void setPaused(bool inPause);

    bool isPaused() const;

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    bool canMove(MoveDirection inDirection);

    virtual bool move(MoveDirection inDirection) = 0;

    bool rotate();

    void dropWithoutCommit();

    void dropAndCommit();

    int level() const;

    void setStartingLevel(int inLevel);

    const Block & activeBlock() const;

    const Grid & gameGrid() const;

    std::size_t currentBlockIndex() const;

    int futureBlocksCount() const;

    void setFutureBlocksCount(int inFutureBlocksCount);

    void getFutureBlocks(std::size_t inCount, BlockTypes & outBlocks);

    void getFutureBlocksWithOffset(std::size_t inOffset, std::size_t inCount, BlockTypes & outBlocks);

    virtual const GameState & gameState() const = 0;

    // For multiplayer crazyness
    virtual void applyLinePenalty(int inNumberOfLinesMadeByOpponent);
    //virtual void setActiveBlock(const Block & inBlock);
    virtual void setGrid(const Grid & inGrid) = 0;
    //void swapGrid(Game & other);
    //void swapActiveBlock(Game & other);

protected:
    static int GetRowDelta(MoveDirection inDirection);
    static int GetColumnDelta(MoveDirection inDirection);

    virtual GameState & gameState() = 0;

    void onChanged();
    void onLinesCleared(int inLineCount);

    static bool Exists(GameImpl * inGame);
    static void OnChangedImpl(GameImpl * inGame);
    static void OnLinesClearedImpl(GameImpl * inGame, int inLineCount);

    static std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, std::size_t inNumColumns);
    void reserveBlocks(std::size_t inCount);
    void supplyBlocks();

    std::vector<BlockType> getGarbageRow() const;

    std::size_t mNumRows;
    std::size_t mNumColumns;
    boost::scoped_ptr<Block> mActiveBlock;
    boost::scoped_ptr<BlockFactory> mBlockFactory;
    BlockTypes mBlocks;
    int mFutureBlocksCount;
    std::size_t mCurrentBlockIndex;
    int mStartingLevel;
    bool mPaused;

    typedef std::set<EventHandler*> EventHandlers;
    EventHandlers mEventHandlers;
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

private:
    // non-copyable
    GameImpl(const GameImpl&);
    GameImpl& operator=(const GameImpl&);

    static bool Exists(const GameImpl & inGame);

    typedef std::set<const GameImpl*> Instances;
    static Instances sInstances;
};


class HumanGame : public GameImpl
{
public:
    inline static ThreadSafe<GameImpl> Create(std::size_t inNumRows, std::size_t inNumColumns)
    {
        return ThreadSafe<GameImpl>(new HumanGame(inNumRows, inNumColumns));
    }

    virtual bool move(MoveDirection inDirection);

    const GameState & gameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    // Friendship required for constructor.
    friend class GameImpl;

    HumanGame(std::size_t inNumRows, std::size_t inNumCols);

    HumanGame(const GameImpl & inGame);

    GameState & gameState();

private:
    boost::scoped_ptr<GameState> mGameState;
};


class ComputerGame : public GameImpl
{
public:
    inline static ThreadSafe<GameImpl> Create(std::size_t inNumRows, std::size_t inNumColumns)
    {
        return ThreadSafe<GameImpl>(new ComputerGame(inNumRows, inNumColumns));
    }

    virtual bool move(MoveDirection inDirection);

    void appendPrecalculatedNode(NodePtr inNode);

    const GameStateNode * currentNode() const;

    const GameStateNode * endNode() const;

    bool navigateNodeDown();

    std::size_t numPrecalculatedMoves() const;

    void clearPrecalculatedNodes();

    const GameState & gameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    // Friendship required for constructor.
    friend class GameImpl;

    ComputerGame(std::size_t inNumRows, std::size_t inNumCols);

    GameState & gameState();

private:
    void setCurrentNode(NodePtr inCurrentNode);

    NodePtr mCurrentNode;
};


} // namespace Tetris


#endif // GAME_H_INCLUDED
