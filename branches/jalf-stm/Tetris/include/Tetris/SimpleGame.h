#ifndef TETRIS_SIMPLEGAME_H
#define TETRIS_SIMPLEGAME_H


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


class GameState;


/**
 * SimpleGame is a simplified wrapper for the Game class.
 */
class SimpleGame
{
public:
    /**
     * EventHandler allows you to listen to game events.
     */
    class EventHandler
    {
    public:
        EventHandler() {}

        virtual void onGameStateChanged(SimpleGame * inGame) = 0;

        virtual void onLinesCleared(SimpleGame * inGame, std::size_t inLineCount) = 0;

    private:
        EventHandler(const EventHandler&);
        EventHandler& operator=(const EventHandler&);
    };

    SimpleGame(PlayerType inPlayerType,
               std::size_t inRowCount,
               std::size_t inColumnCount);

    ~SimpleGame();

    void registerEventHandler(EventHandler * inEventHandler);

    void unregisterEventHandler(EventHandler * inEventHandler);

    bool checkPositionValid(const Block & inBlock) const;

    PlayerType playerType() const;

    GameStateStats stats() const;

    void setPaused(bool inPaused);

    bool isPaused() const;

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    bool move(Direction inDirection);

    bool rotate();

    void drop();

    void setStartingLevel(int inLevel);

    int level() const;

    // Returns a copy to avoid race conditions.
    Block activeBlock() const;

    // Returns a copy to avoid race conditions.
    Grid gameGrid() const;

    // Get the next block
    Block getNextBlock() const;

    // Gets the next scheduled blocks.
    std::vector<Block> getNextBlocks(std::size_t inCount) const;

    // For multiplayer crazyness.
    void applyLinePenalty(int inNumberOfLinesMadeByOpponent);

private:
    friend class ComputerPlayer;
    

    // NOTE: This is thread-unsafe!
    const GameState & gameState() const;

    //GameState & gameState();

    struct Impl;
    boost::shared_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H
