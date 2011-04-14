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


using Futile::ThreadSafe;


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

    Game(size_t inNumRows, size_t inNumColumns);

    virtual ~Game();

    static void RegisterEventHandler(ThreadSafe<Game> inGame, EventHandler * inEventHandler);

    static void UnregisterEventHandler(ThreadSafe<Game> inGame, EventHandler * inEventHandler);

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
    //virtual void setActiveBlock(const Block & inBlock);
    virtual void setGrid(const Grid & inGrid) = 0;
    //void swapGrid(Game & other);
    //void swapActiveBlock(Game & other);

protected:
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
    Futile::AtomicPrimitive<bool> mIsChanged;

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
    HumanGame(size_t inNumRows, size_t inNumCols);

    HumanGame(const Game & inGame);

    virtual bool move(MoveDirection inDirection);

    const GameState & gameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    GameState & gameState();

private:
    boost::scoped_ptr<GameState> mGameState;
};


class ComputerGame : public Game
{
public:
    ComputerGame(size_t inNumRows, size_t inNumCols);

    ComputerGame(const Game & inGame);

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
    GameState & gameState();

private:
    void setCurrentNode(NodePtr inCurrentNode);

    NodePtr mCurrentNode;
};


} // namespace Tetris


#endif // GAME_H_INCLUDED
