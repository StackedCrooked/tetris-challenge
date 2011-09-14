#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


#include "Tetris/BlockMover.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Player.h"
#include "Futile/Threading.h"
#include "Futile/Timer.h"


namespace Tetris {


class GameImpl;
class GameState;


class ComputerPlayer : public Player
{
public:
    class Tweaker
    {
    public:
        /**
         * Gets updated parameters from the tweaker object.
         * This message will be called in the main thread.
         */
        virtual const Evaluator & updateAIParameters(const Player & inPlayer,
                                                     int & outSearchDepth,
                                                     int & outSearchWidth,
                                                     int & outWorkerCount,
                                                     int & outMoveSpeed,
                                                     BlockMover::MoveDownBehavior & outMoveDownBehavior) = 0;
    };

    static PlayerPtr Create(const TeamName & inTeamName,
                            const PlayerName & inPlayerName,
                            std::size_t inRowCount,
                            std::size_t inColumnCount);

    virtual ~ComputerPlayer();

    void setTweaker(Tweaker * inTweaker);

    int searchDepth() const;

    void setSearchDepth(int inSearchDepth);

    // Get progress
    int depth() const;

    int searchWidth() const;

    void setSearchWidth(int inSearchWidth);

    int moveSpeed() const;

    void setMoveSpeed(int inMoveSpeed);

    int workerCount() const;

    // Set to 0 to auto-select
    void setWorkerCount(int inWorkerCount);

private:
    ComputerPlayer(const TeamName & inTeamName,
                   const PlayerName & inPlayerName,
                   std::size_t inRowCount,
                   std::size_t inColumnCount);

    void onTimerEvent();


    struct Impl;
    friend struct Impl;
    Futile::ThreadSafe<Impl> mImpl;
    boost::scoped_ptr<Futile::Timer> mTimer;
};

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
