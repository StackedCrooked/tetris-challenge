#ifndef TETRIS_GRAVITY_H_INCLUDED
#define TETRIS_GRAVITY_H_INCLUDED


#include "Futile/Threading.h"
#include <boost/scoped_ptr.hpp>


namespace Poco {
    class Timer;
}


namespace Tetris {


class GameImpl;
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
    Gravity(const Futile::ThreadSafe<GameImpl> & inGame);

    ~Gravity();

    // Number of rows per second
    double speed() const;

    static double CalculateSpeed(int inLevel);

private:
    Gravity(const Gravity &);
    Gravity & operator=(const Gravity &);

    void onTimerEvent(Poco::Timer & timer);

    struct Impl;
    Futile::ThreadSafe<Impl> mImpl;
    boost::scoped_ptr<Poco::Timer> mTimer;
};


} // namespace Tetris


#endif // GRAVITY_H_INCLUDED

