#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


#include <memory>


namespace Tetris
{
    
    class Evaluator;
    class Game;
    template<class Variable> class Protected;


    class ComputerPlayerImpl;


    class ComputerPlayer
    {
    public:
        ComputerPlayer(const Protected<Game> & inProtectedGame,
                       std::auto_ptr<Evaluator> inEvaluator,
                       int inSearchDepth,
                       int inSearchWidth);

        int searchDepth() const;

        void setSearchDepth(int inSearchDepth);

        // Get progress
        int currentSearchDepth() const;

        int searchWidth() const;

        void setSearchWidth(int inSearchWidth);

        int moveSpeed() const;

        void setMoveSpeed(int inMoveSpeed);

        void setEvaluator(std::auto_ptr<Evaluator> inEvaluator);

        const Evaluator & evaluator() const;

    private:
        ComputerPlayer(const ComputerPlayer &);
        ComputerPlayer & operator= (const ComputerPlayer&);

        std::auto_ptr<ComputerPlayerImpl> mImpl;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
