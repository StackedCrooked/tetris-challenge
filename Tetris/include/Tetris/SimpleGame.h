#ifndef TETRIS_SIMPLEGAME_H_INCLUDED
#define TETRIS_SIMPLEGAME_H_INCLUDED


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include <boost/signals2.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <cstddef>


namespace Tetris {


class Game;
class GameState;
template<class Variable> class ThreadSafe;


/**
 * SimpleGame is an easy to use and thread-safe wrapper for the Game class.
 */
class SimpleGame
{
public:
    boost::signals2::signal<void()> OnChanged;

    boost::signals2::signal<void(int)> OnLinesCleared;

    SimpleGame(size_t inRowCount, size_t inColumnCount);

    ~SimpleGame();

    const ThreadSafe<Game> & game() const;

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    void move(MoveDirection inDirection);

    void rotate();

    void drop();

    int level() const;

    // Returns a copy to avoid race conditions.
    Block activeBlock() const;

    // Returns a copy to avoid race conditions.
    Grid gameGrid() const;

    // Gets the currently active block and any blocks that follow.
    Block getNextBlock() const;

    // For multiplayer crazyness.
    void setActiveBlock(const Block & inBlock);
    void setGameGrid(const Grid & inGrid);

private:
    // non-copyable
    SimpleGame(const SimpleGame & );
    SimpleGame & operator=(const SimpleGame&);

    struct Impl;
    Impl * mImpl;
};

} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H_INCLUDED
