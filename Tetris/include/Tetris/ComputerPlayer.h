#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


#include "Tetris/AutoPtrSupport.h"
#include "Tetris/BlockMover.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Threading.h"


namespace Poco {
class Timer;
}


namespace Tetris {


class Game;
class GameState;


class ComputerPlayer
{
public:
    class Tweaker
    {
    public:
        // WARNING: This callback will be received in a worker thread!
        virtual std::auto_ptr<Evaluator> updateAIParameters(const GameState & inGameState,
                                                            int & outSearchDepth,
                                                            int & outSearchWidth,
                                                            int & outWorkerCount,
                                                            int & outMoveSpeed,
                                                            BlockMover::MoveDownBehavior & outMoveDownBehavior) = 0;
    };

    ComputerPlayer(const std::string & inName,
                   const ThreadSafe<Game> & inProtectedGame,
                   std::auto_ptr<Evaluator> inEvaluator);

    ~ComputerPlayer();

    const std::string & name() const;

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

    //const Evaluator & evaluator() const;

    int workerCount() const;

    // Set to 0 to auto-select (75% of CPU count)
    void setWorkerCount(int inWorkerCount);

private:
    ComputerPlayer(const ComputerPlayer &);
    ComputerPlayer & operator= (const ComputerPlayer&);

    void onTimerEvent(Poco::Timer & );

    struct Impl;
    Impl * mImpl;
};

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
