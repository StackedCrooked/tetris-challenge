#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


#include "Tetris/BlockMover.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Player.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Threading.h"
#include <boost/scoped_ptr.hpp>


namespace Poco {
class Timer;
}


namespace Tetris {


class Game;
class GameState;


class ComputerPlayer : public Player
{
public:
    class Tweaker
    {
    public:
        /**
         * Gets new parametrs from the tweaker object.
         *
         * WARNING: This callback will be received in a worker thread!
         */
        virtual std::auto_ptr<Evaluator> updateAIParameters(const Player & inPlayer,
                                                            int & outSearchDepth,
                                                            int & outSearchWidth,
                                                            int & outWorkerCount,
                                                            int & outMoveSpeed,
                                                            BlockMover::MoveDownBehavior & outMoveDownBehavior) = 0;
    };

    ComputerPlayer(const TeamName & inTeamName,
                   const PlayerName & inPlayerName,
                   size_t inRowCount,
                   size_t inColumnCount);

    virtual ~ComputerPlayer();

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

    int workerCount() const;

    // Set to 0 to auto-select
    void setWorkerCount(int inWorkerCount);

private:
    ComputerPlayer(const ComputerPlayer &);
    ComputerPlayer & operator= (const ComputerPlayer&);

    void onTimerEvent(Poco::Timer & );

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
