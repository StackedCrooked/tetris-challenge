#ifndef TETRIS_SIMPLEGAME_H_INCLUDED
#define TETRIS_SIMPLEGAME_H_INCLUDED


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include "Tetris/GameStateStats.h"
#include "Tetris/PlayerType.h"
#include "Futile/Threading.h"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <vector>


namespace Tetris {


class Game;
class GameState;


/**
 * SimpleGame provides a "simple" interface to the more advanced Game class.
 *
 * Simplifications include:
 * - No need locking
 * - Event callbacks always arrive in the main thread.
 * - Returns game attributes by value (copy) to avoid race conditions.
 */
class SimpleGame
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

        virtual void onGameStateChanged(SimpleGame * inGame) = 0;

        virtual void onLinesCleared(SimpleGame * inGame, int inLineCount) = 0;

    private:
        EventHandler(const EventHandler&);
        EventHandler& operator=(const EventHandler&);
    };

    SimpleGame(PlayerType inPlayerType, std::size_t inRowCount, std::size_t inColumnCount);

    ~SimpleGame();

    void registerEventHandler(EventHandler * inEventHandler);

    void unregisterEventHandler(EventHandler * inEventHandler);

    PlayerType playerType() const;

    GameStateStats stats() const;

    Futile::ThreadSafe<Game> gameImpl() const;

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
    std::size_t futureBlocksCount() const;

    // For multiplayer crazyness.
    void applyLinePenalty(int inNumberOfLinesMadeByOpponent);

private:
    struct Impl;
    boost::shared_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H_INCLUDED
