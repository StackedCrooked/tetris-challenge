#ifndef TETRIS_SIMPLEGAME_H_INCLUDED
#define TETRIS_SIMPLEGAME_H_INCLUDED


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include "Tetris/GameStateStats.h"
#include "Tetris/PlayerType.h"
#include "Futile/Threading.h"
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <vector>


using Futile::ThreadSafe;


namespace Tetris {


class Game;
class GameState;


/**
 * SimpleGame is an easy to use and thread-safe wrapper for the Game class.
 */
class SimpleGame
{
public:
    /**
     * EventHandler allows you to register callback method for game events.
     * All callbacks are invoked on the main thread.
     */
    class EventHandler
    {
    public:
        EventHandler() {}

        virtual void onGameStateChanged(SimpleGame * inGame) = 0;

        virtual void onLinesCleared(SimpleGame * inGame, int inLineCount) = 0;

    private:
        EventHandler(const EventHandler&);
        EventHandler& operator=(const EventHandler&);
    };

    SimpleGame(PlayerType inPlayerType, size_t inRowCount, size_t inColumnCount);

    ~SimpleGame();

    static void RegisterEventHandler(SimpleGame * inSimpleGame, EventHandler * inEventHandler);

    static void UnregisterEventHandler(SimpleGame * inSimpleGame, EventHandler * inEventHandler);

    static bool Exists(SimpleGame * inSimpleGame);

    PlayerType playerType() const;

    GameStateStats stats() const;

    ThreadSafe<Game> game() const;

    void setPaused(bool inPaused);

    bool isPaused() const;

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    void move(MoveDirection inDirection);

    void rotate();

    void drop();

    void setStartingLevel(int inLevel);

    int level() const;

    // Returns a copy to avoid race conditions.
    Block activeBlock() const;

    // Returns a copy to avoid race conditions.
    Grid gameGrid() const;

    // Gets the currently active block and any blocks that follow.
    Block getNextBlock();

    // Gets the next blocks. The length of the resulting vector is the same as futureBlocksCount().
    std::vector<Block> getNextBlocks();

    int futureBlocksCount() const;

    // For multiplayer crazyness.
    void applyLinePenalty(int inNumberOfLinesMadeByOpponent);
    //void setActiveBlock(const Block & inBlock);
    //void setGameGrid(const Grid & inGrid);

private:
    // non-copyable
    SimpleGame(const SimpleGame & );
    SimpleGame & operator=(const SimpleGame&);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;

    typedef std::set<SimpleGame*> Instances;
    static Instances sInstances;
};


} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H_INCLUDED
