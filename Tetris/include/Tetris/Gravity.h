#ifndef TETRIS_GRAVITY_H_INCLUDED
#define TETRIS_GRAVITY_H_INCLUDED


#include "Tetris/Threading.h"
#include <boost/scoped_ptr.hpp>


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
    Gravity(const ThreadSafe<Game> & inGame);

    ~Gravity();

    // Number of rows per second
    double speed() const;

    static double CalculateSpeed(int inLevel);

private:
    Gravity(const Gravity &);
    Gravity & operator=(const Gravity &);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // GRAVITY_H_INCLUDED

