#ifndef TETRIS_GRAVITY_H_INCLUDED
#define TETRIS_GRAVITY_H_INCLUDED


#include <memory>


namespace Tetris
{

    template<class Variable> class Protected;
    class Game;
    class GravityImpl;


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

        // Number of rows per second
        float currentSpeed() const;

        static float CalculateSpeed(int inLevel);

    private:
        Gravity(const Gravity &);
        Gravity & operator=(const Gravity &);

        GravityImpl * mImpl;
    };

} // namespace Tetris


#endif // GRAVITY_H_INCLUDED
