#ifndef TETRIS_COMPUTER_H
#define TETRIS_COMPUTER_H


#include "Futile/Timer.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>


namespace Tetris {


class Game;
class GameState;

class Computer : boost::noncopyable
{
public:
    Computer(Game & inGame);

    ~Computer();

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
};


} // namespace Tetris


#endif // TETRIS_PLAYER_H
