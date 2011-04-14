#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include "Futile/Threading.h"
#include <boost/scoped_ptr.hpp>
#include <memory>
#include <set>


namespace Tetris {


class Block;
class GameStateNode;
class GameImpl;


/**
 * Game is a top-level class for the Tetris core. It manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - the root gamestate node
 */
class Game
{
public:
    class EventHandler
    {
    public:
        EventHandler();

        virtual ~EventHandler();

        // Check if the give EventHandler object still exists
        static bool Exists(EventHandler * inEventHandler);

        virtual void onGameStateChanged(Game * inGame) = 0;

        virtual void onLinesCleared(Game * inGame, int inLineCount) = 0;

    private:
        EventHandler(const EventHandler&);
        EventHandler& operator=(const EventHandler&);

        typedef std::set<EventHandler*> Instances;
        static Instances sInstances;
    };

    /// Factory method.
    template<class SubType>
    static Futile::ThreadSafe<Game> Create(size_t inNumRows, size_t inNumCols)
    {
        // Create the instance
        Futile::ThreadSafe<Game> result(new SubType(inNumRows, inNumCols));

        // Pass the ThreadSafe<Game> to the Game object itself.
        // The ThreadSafe<Game> object is required during event
        // handling.
        Futile::ScopedWriter<Game> writer(result);
        writer->setThreadSafeGame(result);
        return result;
    }

    virtual ~Game();

    static void RegisterEventHandler(Futile::ThreadSafe<Game> inGame, EventHandler * inEventHandler);

    static void UnregisterEventHandler(Futile::ThreadSafe<Game> inGame, EventHandler * inEventHandler);

    void setPaused(bool inPause);

    bool isPaused() const;

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    virtual bool move(MoveDirection inDirection) = 0;

    bool rotate();

    void drop();

    int level() const;

    void setStartingLevel(int inLevel);

    const Block & activeBlock() const;

    const Grid & gameGrid() const;

    size_t currentBlockIndex() const;

    int futureBlocksCount() const;

    void setFutureBlocksCount(int inFutureBlocksCount);

    void getFutureBlocks(size_t inCount, BlockTypes & outBlocks);

    void getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks);

    virtual const GameState & gameState() const = 0;

    // For multiplayer crazyness
    virtual void applyLinePenalty(int inNumberOfLinesMadeByOpponent);
    virtual void setGrid(const Grid & inGrid) = 0;

protected:
    Game(size_t inNumRows, size_t inNumColumns);

    void setThreadSafeGame(const Futile::ThreadSafe<Game> & inThreadSafeGame);

    virtual GameState & gameState() = 0;

    void onChanged();
    void onLinesCleared(int inLineCount);

    static bool Exists(Game * inGame);
    static void OnChangedImpl(Game * inGame);
    static void OnLinesClearedImpl(Game * inGame, int inLineCount);

    static std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns);
    void reserveBlocks(size_t inCount);
    void supplyBlocks();

    std::vector<BlockType> getGarbageRow() const;

    size_t mNumRows;
    size_t mNumColumns;
    boost::scoped_ptr<Block> mActiveBlock;
    boost::scoped_ptr<BlockFactory> mBlockFactory;
    BlockTypes mBlocks;
    int mFutureBlocksCount;
    size_t mCurrentBlockIndex;
    int mStartingLevel;
    bool mPaused;
    bool mIsChanged;
    boost::scoped_ptr< Futile::ThreadSafe<Game> > mThreadSafeGame;

    typedef std::set<EventHandler*> EventHandlers;
    EventHandlers mEventHandlers;

private:
    // non-copyable
    Game(const Game&);
    Game& operator=(const Game&);

    typedef std::set<Game*> Instances;
    static Instances sInstances;
};


class HumanGame : public Game
{
public:
    virtual bool move(MoveDirection inDirection);

    const GameState & gameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    friend class Game;

    HumanGame(size_t inNumRows, size_t inNumCols);

    HumanGame(const Game & inGame);

    GameState & gameState();

private:
    boost::scoped_ptr<GameState> mGameState;
};


class ComputerGame : public Game
{
public:
    virtual bool move(MoveDirection inDirection);

    void appendPrecalculatedNode(NodePtr inNode);

    const GameStateNode * currentNode() const;

    const GameStateNode * endNode() const;

    bool navigateNodeDown();

    size_t numPrecalculatedMoves() const;

    void clearPrecalculatedNodes();

    const GameState & gameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    friend class Game;

    ComputerGame(size_t inNumRows, size_t inNumCols);

    ComputerGame(const Game & inGame);

    GameState & gameState();

private:
    void setCurrentNode(NodePtr inCurrentNode);

    NodePtr mCurrentNode;
};


} // namespace Tetris


#endif // GAME_H_INCLUDED
