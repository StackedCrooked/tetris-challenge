#ifndef TETRIS_GRAVITY_H_INCLUDED
#define TETRIS_GRAVITY_H_INCLUDED


#include "Tetris/Utilities.h"
#include <memory>
#include <boost/function.hpp>


namespace Tetris
{

    template<class Variable> class Protected;
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
        Gravity(const Protected<Game> & inGame);

        ~Gravity();

        // Receive notifications each time a block has been lowered.
        // Allows the reciever to update the view etc..
        void setCallback(const GravityCallback & inGravityCallback);

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

