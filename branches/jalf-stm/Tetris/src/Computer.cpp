#include "Tetris/Computer.h"
#include "Tetris/AISupport.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/GameState.h"
#include "Tetris/NodeCalculator.h"
#include "Futile/Logging.h"
#include "Futile/Timer.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
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
        mNumMovesPerSecond(10),
        mSearchDepth(4),
        mSearchWidth(4),
        mWorkerCount(4),
        mSyncError(false),
        mWorker("Computer"),
        mWorkerPool("Computer", 4),
        mMoveTimer(20),
        mCoordinationTimer(50)
    {
    }

    ~Impl()
    {
    }

    void move();

    static Game::MoveResult move(stm::transaction & tx, Game & ioGame, const Block & targetBlock);

    void coordinate();

    typedef std::vector<GameState> GameStates;
    typedef GameStates Precalculated;
    typedef GameStates Preliminaries;

    Game & mGame;
    mutable stm::shared<Precalculated> mPrecalculated;
    mutable stm::shared<unsigned> mNumMovesPerSecond;
    mutable stm::shared<unsigned> mSearchDepth;
    mutable stm::shared<unsigned> mSearchWidth;
    mutable stm::shared<unsigned> mWorkerCount;
    mutable stm::shared<bool> mSyncError;
    Worker mWorker;
    WorkerPool mWorkerPool;
    boost::shared_ptr<NodeCalculator> mNodeCalculator;
    Futile::Timer mMoveTimer;
    Futile::Timer mCoordinationTimer;
};


void Computer::Impl::coordinate()
{
    if (get(mSyncError))
    {
        mNodeCalculator.reset();
        set(mSyncError, false);
    }

    if (get(mPrecalculated).size() > 4)
    {
        return;
    }

    Preliminaries prelim;
    if (mNodeCalculator)
    {
        prelim = mNodeCalculator->getCurrentResults();
        if (prelim.empty())
        {
            return;
        }
        LogDebug(SS() << mNodeCalculator->getCurrentNodeCount() << ": " << mNodeCalculator->getCurrentSearchDepth() << "/" << mNodeCalculator->getMaxSearchDepth());
    }


    BlockTypes blockTypes;
    boost::scoped_ptr<GameState> lastGameState;

    stm::atomic([&](stm::transaction & tx)
    {
        const Precalculated & cPrec = mPrecalculated.open_r(tx);
        if (!cPrec.empty())
        {
            return;
        }

        Precalculated & prec = mPrecalculated.open_rw(tx);
        prec.insert(prec.end(), prelim.begin(), prelim.end());

        blockTypes = mGame.getFutureBlocks(tx, prec.size() + 8);
        blockTypes.erase(blockTypes.begin(), blockTypes.begin() + prec.size());
        lastGameState.reset(new GameState(prec.empty() ? mGame.gameState(tx) : prec.back()));
    });

    if (!blockTypes.empty())
    {
        Widths widths(blockTypes.size(), 4);
        Assert(widths.size() == blockTypes.size());
        const Evaluator & evaluator = MakeTetrises::Instance();
        mNodeCalculator.reset(new NodeCalculator(*lastGameState,
                                                 blockTypes,
                                                 widths,
                                                 evaluator,
                                                 mWorker,
                                                 mWorkerPool));

        mNodeCalculator->start();
    }
}


Game::MoveResult Computer::Impl::move(stm::transaction & tx, Game & ioGame, const Block & targetBlock)
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
        Precalculated & prec = mPrecalculated.open_rw(tx);
        if (prec.empty())
        {
            return;
        }

        if (prec.front().originalBlock().type() == mGame.activeBlock(tx).type())
        {
            if (Game::MoveResult_Commited == move(tx, mGame, prec.front().originalBlock()))
            {
                prec.erase(prec.begin());
            }
        }
        else
        {
            mPrecalculated.open_rw(tx).clear();
            set(mSyncError, true);
        }
    });
}


Computer::Computer(Game &inGame) :
    mImpl(new Impl(inGame))
{
    mImpl->mMoveTimer.start(boost::bind(&Computer::Impl::move, mImpl.get()));
    mImpl->mCoordinationTimer.start(boost::bind(&Computer::Impl::coordinate, mImpl.get()));
}


Computer::~Computer()
{
    mImpl->mCoordinationTimer.stop();
    mImpl->mMoveTimer.stop();
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
