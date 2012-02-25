#ifndef TETRIS_COMPUTERPLAYER_H
#define TETRIS_COMPUTERPLAYER_H


#include "Tetris/Evaluator.h"
#include "Tetris/Player.h"
#include "Futile/Threading.h"
#include "Futile/Timer.h"


namespace Tetris {


class Game;
class GameState;

class Computer
{
public:
    Computer(Game & inGame);

    virtual ~Computer();

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
    void onTimerEvent();
    void onMoveTimerEvent();

    struct Impl;
    friend struct Impl;
    boost::scoped_ptr<Impl> mImpl;
    boost::scoped_ptr<Futile::Timer> mTimer;
    boost::scoped_ptr<Futile::Timer> mMoveTimer;
};


class ComputerPlayer : public Player,
                       public Computer

{
public:
    static PlayerPtr Create(const TeamName & inTeamName,
                                   const PlayerName & inPlayerName,
                                   std::size_t inRowCount,
                                   std::size_t inColumnCount)
    {
        PlayerPtr result(new ComputerPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
        return result;
    }

    ComputerPlayer(const TeamName & inTeamName,
                   const PlayerName & inPlayerName,
                   std::size_t inRowCount,
                   std::size_t inColumnCount);

    virtual ~ComputerPlayer() {}
};


} // namespace Tetris


#endif // TETRIS_PLAYER_H
