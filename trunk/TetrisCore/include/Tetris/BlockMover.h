#ifndef TETRIS_BLOCKMOVER_H_INCLUDED
#define TETRIS_BLOCKMOVER_H_INCLUDED


namespace Tetris
{

    template<class Variable> class Protected;
    class Game;


    class BlockMoverImpl;


    /**
     * BlockMover
     *
     * A BlockMove object will peridically move the current tetris block
     * one square closer to the next precalculated block (from the AI).
     *
     * If no precalculated blocks are available then it will enter a waiting state.
     */
    class BlockMover
    {
    public:
        BlockMover(const Protected<Game> & inGame, int inInterval);

        ~BlockMover();

        void setSpeed(int inNumMovesPerSecond);

        int speed() const;

        void setInterval(int inTimeBetweenMovesInMilliseconds);

        int interval() const;

    private:
        BlockMover(const BlockMover &);
        BlockMover & operator=(const BlockMover&);

        BlockMoverImpl * mImpl;
    };

} // namespace Tetris


#endif // BLOCKMOVER_H_INCLUDED
