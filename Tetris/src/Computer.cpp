#include "Tetris/Computer.h"
#include "Tetris/Game.h"


namespace {


template<typename T>
static void set(stm::shared<T> & dst, const T & val)
{
    stm::atomic([&](stm::transaction & tx){ dst.open_rw(tx) = val; });
}

template<typename T>
static T get(stm::shared<T> & src)
{
    return stm::atomic<T>([&](stm::transaction & tx){ return src.open_r(tx); });
}


} // anonymous namespace


namespace Tetris {


struct Computer::Impl : boost::noncopyable
{
    Impl(Game & inGame) :
        mGame(inGame),
        mNumMovesPerSecond(50),
        mSearchDepth(4),
        mSearchWidth(4),
        mWorkerCount(4)
    {
    }

    ~Impl()
    {
    }

    Game & mGame;
    mutable stm::shared<unsigned> mNumMovesPerSecond;
    mutable stm::shared<unsigned> mSearchDepth;
    mutable stm::shared<unsigned> mSearchWidth;
    mutable stm::shared<unsigned> mWorkerCount;
};


Computer::Computer(Game &inGame) :
    mImpl(new Impl(inGame))
{
}


Computer::~Computer()
{
    mImpl.reset();
}


void Computer::setSearchDepth(unsigned inSearchDepth)
{
    set(mImpl->mNumMovesPerSecond, inSearchDepth);
}


unsigned Computer::searchDepth() const
{
    return get(mImpl->mSearchDepth);
}


void Computer::setSearchWidth(unsigned inSearchWidth)
{
    set(mImpl->mSearchWidth, inSearchWidth);
}


unsigned Computer::searchWidth() const
{
    return get(mImpl->mSearchWidth);
}


void Computer::setWorkerCount(unsigned inWorkerCount)
{
    set(mImpl->mWorkerCount, inWorkerCount);
}


unsigned Computer::workerCount() const
{
    return get(mImpl->mWorkerCount);
}


void Computer::setMoveSpeed(unsigned inMovesPerSecond)
{
    set(mImpl->mNumMovesPerSecond, inMovesPerSecond);
}


unsigned Computer::moveSpeed() const
{
    return get(mImpl->mNumMovesPerSecond);
}


} // namespace Tetris
