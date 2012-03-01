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

    void setSearchDepth(int inSearchDepth);

    int searchDepth() const;

    void setSearchWidth(int inSearchWidth);

    int searchWidth() const;

    void setMoveSpeed(int inMoveSpeed);

    int moveSpeed() const;

    void setWorkerCount(int inWorkerCount);

    int workerCount() const;

private:
    void onMoveTimerEvent();

    struct Impl;
    friend struct Impl;
    boost::scoped_ptr<Impl> mImpl;
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
