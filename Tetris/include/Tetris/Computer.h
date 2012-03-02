#ifndef TETRIS_COMPUTER_H
#define TETRIS_COMPUTER_H


#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>


namespace Tetris {


class Game;


class Computer : boost::noncopyable
{
public:
    Computer(Game & inGame);

    ~Computer();

    void setSearchDepth(unsigned inSearchDepth);

    unsigned searchDepth() const;

    void setSearchWidth(unsigned inSearchWidth);

    unsigned searchWidth() const;

    void setWorkerCount(unsigned inWorkerCount);

    unsigned workerCount() const;

    void setMoveSpeed(unsigned inMovesPerSecond);

    unsigned moveSpeed() const;

private:
    void onMoveTimerEvent();

    struct Impl;
    friend struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_PLAYER_H
