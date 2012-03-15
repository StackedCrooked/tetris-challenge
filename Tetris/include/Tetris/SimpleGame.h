#ifndef TETRIS_SIMPLEGAME_H
#define TETRIS_SIMPLEGAME_H


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include "Tetris/GameStateStats.h"
#include "Tetris/PlayerType.h"
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <cstddef>
#include <stdexcept>
#include <vector>


namespace Tetris {


class Game;


/**
 * SimpleGame is a simplified wrapper for the Game class.
 */
class SimpleGame
{
public:
    SimpleGame(PlayerType inPlayerType,
               std::size_t inRowCount,
               std::size_t inColumnCount);

    ~SimpleGame();

    boost::signals2::signal<void(const SimpleGame &)> Changed;
    boost::signals2::signal<void(const SimpleGame &, unsigned)> LinesCleared;

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

    Game & game();

    struct Impl;
    boost::shared_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H
