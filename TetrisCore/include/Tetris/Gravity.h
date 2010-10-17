#ifndef TETRIS_GRAVITY_H_INCLUDED
#define TETRIS_GRAVITY_H_INCLUDED


namespace Tetris
{
    
    template<class Variable> class Protected;
    class Game;


    class GravityImpl;


    /**
     * Gravity
     *
     * Gravity implements the the gravity aspect to the game.
     * It moves the active block one unit down at regular intervals.
     */
    class Gravity
    {
    public:
        Gravity(const Protected<Game> & inGame);

        ~Gravity();

        int getLevel() const;

        // Number of rows per second
        float currentSpeed() const;

        // The higher the level the faster the blocks are falling.
        // Levels go from 0 to 20. If the level is set to a
        // bigger value then it is set to 20 instead.
        void setLevel(int inLevel);

    private:
        Gravity(const Gravity &);
        Gravity & operator=(const Gravity &);

        GravityImpl * mImpl;
    };

} // namespace Tetris


#endif // GRAVITY_H_INCLUDED
