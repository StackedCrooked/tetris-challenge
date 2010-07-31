#ifndef GAMECONTROLLER_H_INCLUDED
#define GAMECONTROLLER_H_INCLUDED


#include "Direction.h"
#include <boost/scoped_ptr.hpp>


namespace Tetris
{

    class Game;

    class GameController
    {
    public:
        GameController(size_t inNumRows, size_t inNumColumns);

        Game & game();

        const Game & game() const;

        void move(Direction inDirection);

        void rotate();

        void drop();

    private:
        boost::scoped_ptr<Game> mGame;
    };

} // namespace Tetris

#endif // GAMECONTROLLER_H_INCLUDED
