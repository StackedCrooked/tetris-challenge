#ifndef TETRIS_GRAVITY_H_INCLUDED
#define TETRIS_GRAVITY_H_INCLUDED


#include <memory>


namespace Tetris
{

    template<class Variable> class Protected;
    class Game;
    class Gravity;
    class GravityImpl;


    class AbstractGravityCallback
    {
    public:
        virtual void operator()(Gravity * inGravity) = 0;
    };


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
        void setGravityCallback(AbstractGravityCallback * inGravityCallback);

        // Number of rows per second
        double speed() const;

        static double CalculateSpeed(int inLevel);

    private:
        Gravity(const Gravity &);
        Gravity & operator=(const Gravity &);

        GravityImpl * mImpl;
    };



    /**
     * GravityCallback is can be used as functor for receiving gravity callbacks.
     */
    template<class T>
    class GravityCallback : public AbstractGravityCallback
    {
    public:
        GravityCallback(T * inObj) :
            mObj(inObj)
        {
        }


        virtual void operator()(Gravity * inGravity)
        {
            mObj->onGravityCallback(inGravity);
        }

    private:
        T * mObj;
    };

} // namespace Tetris


#endif // GRAVITY_H_INCLUDED

