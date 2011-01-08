#ifndef TETRIS_SIMPLEGAME_H_INCLUDED
#define TETRIS_SIMPLEGAME_H_INCLUDED


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <cstddef>


namespace Tetris {


/**
 * SimpleGame is an easy to use and thread-safe wrapper for the Game class.
 */
class SimpleGame
{
public:
    class EventHandler
    {
    public:
        virtual void onSimpleGameChanged() = 0;
    };

    SimpleGame(SimpleGame::EventHandler * inEventHandler, size_t inRowCount, size_t inColumnCount);

    ~SimpleGame();

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    void move(Direction inDirection);

    void rotate();

    void drop();

    int level() const;

    // Returns a copy to avoid race conditions.
    Block activeBlock() const;

    // Returns a copy to avoid race conditions.
    Grid gameGrid() const;

    // Gets the currently active block and any blocks that follow.
    Block getNextBlock() const;

private:
    // non-copyable
    SimpleGame(const SimpleGame & );
    SimpleGame & operator=(const SimpleGame&);

    struct SimpleGameImpl;
    SimpleGameImpl * mImpl;
};

} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H_INCLUDED
