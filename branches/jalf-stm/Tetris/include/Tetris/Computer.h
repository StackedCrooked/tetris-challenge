#ifndef TETRIS_COMPUTER_H
#define TETRIS_COMPUTER_H


#include "Futile/Timer.h"
#include <boost/scoped_ptr.hpp>


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

    void setMoveSpeed(unsigned inMoveSpeed);

    unsigned moveSpeed() const;

    void setWorkerCount(int inWorkerCount);

    int workerCount() const;

private:
    void onMoveTimerEvent();

    struct Impl;
    friend struct Impl;
    boost::scoped_ptr<Impl> mImpl;
    boost::scoped_ptr<Futile::Timer> mMoveTimer;
};


} // namespace Tetris


#endif // TETRIS_PLAYER_H
