#ifndef TETRIS_GRAVITY_H
#define TETRIS_GRAVITY_H


#include <boost/scoped_ptr.hpp>


namespace Futile { class Timer; }


namespace Tetris {


class Game;
class Gravity;


/**
 * Gravity
 *
 * Adds gravity to the game causing the blocks to fall down.
 * The speed at which they fall depends on the game level.
 */
class Gravity
{
public:
    Gravity(Game & inGame);

    ~Gravity();

private:
    Gravity(const Gravity &);
    Gravity & operator=(const Gravity &);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
    boost::scoped_ptr<Futile::Timer> mTimer;
};


} // namespace Tetris


#endif // TETRIS_GRAVITY_H
