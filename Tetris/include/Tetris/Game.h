#ifndef TETRIS_SIMPLEGAME_H_INCLUDED
#define TETRIS_SIMPLEGAME_H_INCLUDED


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include "Tetris/GameStateStats.h"
#include "Tetris/PlayerType.h"
#include "Futile/Threading.h"
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/signals2/signal.hpp>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <vector>


using Futile::ThreadSafe;


namespace Tetris {


class GameImpl;
class GameState;


/**
 * Game is an easy to use and thread-safe wrapper for the GameImpl class.
 */
class Game
{
public:
    /**
     * EventHandler allows you to listen to game events.
     * All callbacks arrive in the main threads.
     */
    class EventHandler
    {
    public:
        EventHandler() {}

        virtual void onGameStateChanged(Game* inGame) = 0;

        virtual void onLinesCleared(Game* inGame, int inLineCount) = 0;

    protected:
        ~EventHandler(){}

    private:
        EventHandler(const EventHandler&);
        EventHandler& operator=(const EventHandler&);
    };

    Game(PlayerType inPlayerType, std::size_t inRowCount, std::size_t inColumnCount);

    ~Game();

    static void RegisterEventHandler(Game* inGame, EventHandler* inEventHandler);

    static void UnregisterEventHandler(Game* inGame, EventHandler* inEventHandler);

    static bool Exists(Game* inGame);

    PlayerType playerType() const;

    GameStateStats stats() const;

    ThreadSafe<GameImpl> gameImpl() const;

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

    // Gets the currently active block.
    Block getNextBlock();

    // Gets the next scheduled blocks.
    std::vector<Block> getNextBlocks();

    // The number next blocks that can be obtained.
    int futureBlocksCount() const;

    // For multiplayer crazyness.
    void applyLinePenalty(int inNumberOfLinesMadeByOpponent);

private:
    // non-copyable
    Game(const Game& );
    Game& operator=(const Game&);

    typedef Futile::Locker<GameImpl> Locker;

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;

    typedef std::set<Game*> Instances;
    static Instances sInstances;
};


} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H_INCLUDED
