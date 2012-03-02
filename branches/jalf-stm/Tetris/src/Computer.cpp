#include "Tetris/Computer.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/GameState.h"
#include "Tetris/NodeCalculator.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include "Futile/Timer.h"
#include <vector>


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


using namespace Futile;


struct Computer::Impl : boost::noncopyable
{
    Impl(Game & inGame) :
        mGame(inGame),
        mPrecalculated(Precalculated()),
        mPreliminaries(Preliminaries()),
        mNumMovesPerSecond(50),
        mSearchDepth(4),
        mSearchWidth(4),
        mWorkerCount(4),
        mTimer(10),
        mWorker("Computer"),
        mWorkerPool("Computer", 4)
    {
    }

    ~Impl()
    {
    }

    void tick();

    void move();

    GameState lastPrecalculatedGameState() const;

    typedef std::vector<GameState> GameStates;
    typedef GameStates Precalculated;
    typedef GameStates Preliminaries;

    Game & mGame;
    mutable stm::shared<Precalculated> mPrecalculated;
    mutable stm::shared<Preliminaries> mPreliminaries;
    mutable stm::shared<unsigned> mNumMovesPerSecond;
    mutable stm::shared<unsigned> mSearchDepth;
    mutable stm::shared<unsigned> mSearchWidth;
    mutable stm::shared<unsigned> mWorkerCount;
    Futile::Timer mTimer;
    Worker mWorker;
    WorkerPool mWorkerPool;
    boost::shared_ptr<NodeCalculator> mNodeCalculator;
};


void Computer::Impl::tick()
{
    move();

    if (get(mPrecalculated).size() >= 4)
    {
        return;
    }

    auto status = mNodeCalculator->status();
    bool restart = false;

    stm::atomic([&](stm::transaction & tx)
    {
        const Preliminaries & prelim = mPreliminaries.open_r(tx);
        if (prelim.size() < 4 && status != NodeCalculator::Status_Finished)
        {
            return;
        }

        Precalculated & prec = mPrecalculated.open_rw(tx);
        for (auto p : prelim)
        {
            prec.push_back(p);
        }

        mPreliminaries.open_rw(tx).clear();
        restart = true;
    });

    if (restart)
    {
        BlockTypes blockTypes;
        Widths widths;
        const Evaluator & evaluator = MakeTetrises::Instance();
        mNodeCalculator.reset(new NodeCalculator(lastPrecalculatedGameState(),
                                                 blockTypes,
                                                 widths,
                                                 evaluator,
                                                 mWorker,
                                                 mWorkerPool));
    }



    if (mNodeCalculator->status() > NodeCalculator::Status_Initial && !get(mPreliminaries).empty())
    {
        // use current results
        // kill node calculator in sep thread
    }
}


GameState Computer::Impl::lastPrecalculatedGameState() const
{
    return stm::atomic<GameState>([&](stm::transaction & tx)
    {
        assert(mPreliminaries.open_r(tx).empty());
        const Precalculated & pc = mPrecalculated.open_r(tx);
        if (!pc.empty())
        {
            return pc.back();
        }
        else
        {
            return mGame.gameState(tx);
        }
    });
}


void Computer::Impl::move()
{
}


Computer::Computer(Game &inGame) :
    mImpl(new Impl(inGame))
{
    mImpl->mTimer.start(boost::bind(&Computer::Impl::tick, mImpl.get()));
}


Computer::~Computer()
{
    mImpl->mTimer.stop();
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
