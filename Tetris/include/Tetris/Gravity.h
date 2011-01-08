#ifndef TETRIS_GRAVITY_H_INCLUDED
#define TETRIS_GRAVITY_H_INCLUDED


#include "Tetris/Utilities.h"
#include <memory>
#include <boost/function.hpp>


namespace Tetris
{

    template<class Variable> class ThreadSafe;
    class Game;
    class Gravity;
    class GravityImpl;
    typedef boost::function<void(Gravity*)> GravityCallback;


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

        GravityImpl * mImpl;
    };

} // namespace Tetris


#endif // GRAVITY_H_INCLUDED

