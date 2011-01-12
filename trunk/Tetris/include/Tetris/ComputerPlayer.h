#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


#include <memory>
#include <boost/function.hpp>


namespace Tetris
{

    class Evaluator;
    class ComputerGame;
    class GameState;
    template<class Variable> class ThreadSafe;


    class ComputerPlayerImpl;


    class ComputerPlayer
    {
    public:
        typedef boost::function<std::auto_ptr<Evaluator>(const GameState &)> GetEvaluatorCallback;

        ComputerPlayer(const ThreadSafe<ComputerGame> & inProtectedGame,
                       std::auto_ptr<Evaluator> inEvaluator,
                       int inSearchDepth,
                       int inSearchWidth,
                       int inWorkerCount);

        ComputerPlayer(const ThreadSafe<ComputerGame> & inProtectedGame,
                       const GetEvaluatorCallback & inGetEvaluator,
                       int inSearchDepth,
                       int inSearchWidth,
                       int inWorkerCount);

        ~ComputerPlayer();

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

        // Enables you to have the Evaluator selected in a callback event.
        // This overrides the evaluator set in the constructor
        // or in the last call to setEvaluator.
        void setDelayedEvaluator(const GetEvaluatorCallback & inGetEvaluatorCallback);

        // Removes the delayed evaluator. Falls back to the regular evaluator from the
        // constructor or last call to setEvaluator().
        void removeDelayedEvaluator();

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
