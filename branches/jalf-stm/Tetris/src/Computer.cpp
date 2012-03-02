#include "Tetris/Computer.h"
#include "Tetris/AISupport.h"
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
    BlockTypes blockTypes;
    boost::scoped_ptr<GameState> lastGameState;

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
        blockTypes = mGame.getFutureBlocks(tx, prec.size() + 8);
        lastGameState.reset(new GameState(prec.back()));
    });

    if (!blockTypes.empty())
    {
        Widths widths(4, blockTypes.size());
        const Evaluator & evaluator = MakeTetrises::Instance();
        mNodeCalculator.reset(new NodeCalculator(*lastGameState,
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


Game::MoveResult Move(stm::transaction & tx, Game & ioGame, const Block & targetBlock)
{
    const Block & block = ioGame.activeBlock(tx);
    Assert(block.type() == targetBlock.type());

    // Try rotation first, if it fails then skip rotation and try horizontal move
    if (block.rotation() != targetBlock.rotation())
    {
        if (ioGame.rotate(tx) == Game::MoveResult_Moved)
        {
            return Game::MoveResult_Moved;
        }
        // else: try left or right move below
    }

    if (block.column() < targetBlock.column())
    {
        if (ioGame.move(tx, MoveDirection_Right) == Game::MoveResult_Moved)
        {
            return Game::MoveResult_Moved;
        }
        else
        {
            // Can't move the block to the left.
            // Our path is blocked.
            // Give up on this block and just drop it.
            ioGame.dropAndCommit(tx);
            return Game::MoveResult_Commited;
        }
    }

    if (block.column() > targetBlock.column())
    {
        if (ioGame.move(tx, MoveDirection_Left) == Game::MoveResult_Moved)
        {
            return Game::MoveResult_Moved;
        }
        else
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            ioGame.dropAndCommit(tx);
            return Game::MoveResult_Commited;
        }
    }

    // Horizontal position is OK.
    // Retry rotation again. If it fails here then drop the block.
    if (block.rotation() != targetBlock.rotation())
    {
        if (ioGame.rotate(tx) == Game::MoveResult_Moved)
        {
            return Game::MoveResult_Moved;
        }
        else
        {
            ioGame.dropAndCommit(tx);
            return Game::MoveResult_Commited;
        }
    }

    return ioGame.move(tx, MoveDirection_Down);
}


void Computer::Impl::move()
{
    stm::atomic([&](stm::transaction & tx)
    {
        Precalculated prec = mPrecalculated.open_r(tx);
        if (prec.empty())
        {
            return;
        }
        Move(tx, mGame, prec.front().originalBlock());
    });
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
