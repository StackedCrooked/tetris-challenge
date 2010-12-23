#ifndef TETRIS_SIMPLEGAME_H_INCLUDED
#define TETRIS_SIMPLEGAME_H_INCLUDED


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Gravity.h"
#include "Tetris/Grid.h"
#include "Tetris/Threading.h"
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <cstddef>


namespace Tetris {


class Game;


/**
 * SimpleGame is an easy to use and thread-safe wrapper for the Game class.
 */
class SimpleGame
{
public:
    SimpleGame(size_t inRowCount, size_t inColumnCount);

    ~SimpleGame();

	// Get access to the game object.
    Protected<Game> & getGame() { return mGame; }

    const Protected<Game> & getGame() const { return mGame; }

    bool isGameOver() const;

    void getSize(int & outColoumCount, int & outRowCount);

    int rowCount() const;

    int columnCount() const;

    void move(Direction inDirection);

    void rotate();

    void drop();

    int level() const;

    // Set to 0 to revert to default speed based on numer of lines made.
    void setLevel(int inLevel);

    // Returns a copy to avoid race conditions.
    Block activeBlock() const;

    // Returns a copy to avoid race conditions.
    Grid gameGrid() const;

    // Gets the currently active block and any blocks that follow.
    std::vector<Block> getNextBlocks(size_t inCount) const;

    void enableGravity(bool inEnabled);

    void enableComputerPlayer(bool inEnabled);

    void setComputerMoveSpeed(int inNumberOfMovesPerSecond);

private:
    // non-copyable
    SimpleGame(const SimpleGame & );
    SimpleGame & operator=(const SimpleGame&);

    Protected<Game> mGame;
    boost::scoped_ptr<Gravity> mGravity;
    boost::scoped_ptr<ComputerPlayer> mComputerPlayer;
    int mComputerMoveSpeed;
};

} // namespace Tetris


#endif // GAME_H_INCLUDED
