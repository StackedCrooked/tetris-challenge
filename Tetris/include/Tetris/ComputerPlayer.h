#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


#include <memory>
#include <boost/function.hpp>


namespace Tetris
{

    class Evaluator;
    class Game;
    class ComputerGame;
    class GameState;
    template<class Variable> class ThreadSafe;


    class ComputerPlayerImpl;


    class ComputerPlayer
    {
    public:
        class Tweaker
        {
        public:
            virtual std::auto_ptr<Evaluator> updateInfo(const GameState & inGameState,
                                                        int & outSearchDepth,
                                                        int & outSearchWidth) = 0;
        };

        ComputerPlayer(const ThreadSafe<Game> & inProtectedGame,
                       std::auto_ptr<Evaluator> inEvaluator,
                       int inSearchDepth,
                       int inSearchWidth,
                       int inWorkerCount);

        ComputerPlayer(const ThreadSafe<Game> & inProtectedGame,
                       int inSearchDepth,
                       int inSearchWidth,
                       int inWorkerCount);

        ~ComputerPlayer();

        void setTweaker(Tweaker * inTweaker);

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

        int workerCount() const;

        // Set to 0 to auto-select (75% of CPU count)
        void setWorkerCount(int inWorkerCount);

    private:
        ComputerPlayer(const ComputerPlayer &);
        ComputerPlayer & operator= (const ComputerPlayer&);

        ComputerPlayerImpl * mImpl;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
