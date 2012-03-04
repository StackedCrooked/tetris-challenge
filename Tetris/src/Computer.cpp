#include "Tetris/Computer.h"
#include "Tetris/AISupport.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/GameState.h"
#include "Tetris/NodeCalculator.h"
#include "Futile/Logging.h"
#include "Futile/STMSupport.h"
#include "Futile/Timer.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include <future>
#include <vector>


namespace Tetris {


using namespace Futile;


struct Computer::Impl : boost::noncopyable
{
    Impl(Game & inGame) :
        mGame(inGame),
        mPrecalculated(Precalculated()),
        mNumMovesPerSecond(25),
        mSearchDepth(10),
        mSearchWidth(2),
        mWorkerCount(16),
        mSyncError(false),
        mWorker("Computer"),
        mWorkerPool("Computer", STM::get(mWorkerCount)),
        mMoveTimer(10),
        mCoordinationTimer(20)
    {
    }

    ~Impl()
    {
    }

    void move();

    Game::MoveResult move(stm::transaction & tx, Game & ioGame, const Block & targetBlock);

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
    if (STM::get(mSyncError))
    {
        LogDebug(SS() << "Sync error. Reset.");
        mNodeCalculator.reset();
        STM::set(mSyncError, false);
    }

    std::size_t numPrecalculated = STM::get(mPrecalculated).size();
    if (numPrecalculated > 4) {
        return;
    }

    Preliminaries preliminaries = mNodeCalculator ? mNodeCalculator->results() : Preliminaries();
    if (mNodeCalculator && preliminaries.empty())
    {
        LogWarning(SS() << "No results yet. Progress is " << mNodeCalculator->progress());
        return;
    }

//    if (mNodeCalculator && preliminaries.size() < 6)
//    {
//        LogDebug(SS() << "Wait a little. preliminaries.size() is only " << preliminaries.size());
//        return;
//    }

    if (mNodeCalculator)
    {
        LogInfo(SS() << "Interrupt at " << mNodeCalculator->progress());
    }

    BlockTypes blockTypes;
    boost::scoped_ptr<GameState> lastGameState;

    stm::atomic([&](stm::transaction & tx)
    {
        Precalculated & precalculated = mPrecalculated.open_rw(tx);
        for (auto prelim : preliminaries)
        {
            if (!precalculated.empty())
            {
                if (prelim.id() <= precalculated.back().id())
                {
                    continue;
                }
                Assert(prelim.id() == precalculated.back().id() + 1);
            }
            else
            {
                if (prelim.id() <= mGame.gameState(tx).id())
                {
                    continue;
                }
                Assert(prelim.id() == mGame.gameState(tx).id() + 1);
            }
            precalculated.push_back(prelim);
        }
        preliminaries.clear();

        blockTypes = mGame.getFutureBlocks(tx, precalculated.size() + mSearchDepth.open_r(tx));
        blockTypes.erase(blockTypes.begin(), blockTypes.begin() + precalculated.size());
        lastGameState.reset(new GameState(precalculated.empty() ? mGame.gameState(tx) : precalculated.back()));
    });

    if (!blockTypes.empty() && !lastGameState->isGameOver())
    {
        Widths widths(blockTypes.size(), STM::get(mSearchWidth));
        Assert(widths.size() == blockTypes.size());
        const GameState & gs = *lastGameState;
        const Evaluator * ev = &MakeTetrises::Instance();
        if (gs.firstOccupiedRow() < 8)
        {
            ev = &Survival::Instance();
        }
        mNodeCalculator.reset(new NodeCalculator(*lastGameState,
                                                 blockTypes,
                                                 widths,
                                                 *ev,
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
            STM::set(mSyncError, true);
            return Game::MoveResult_Committed;
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
            STM::set(mSyncError, true);
            return Game::MoveResult_Committed;
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
            STM::set(mSyncError, true);
            return Game::MoveResult_Committed;
        }
    }

    ioGame.dropWithoutCommit(tx);
    return Game::MoveResult_Moved;
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

        while (!prec.empty() && prec.front().originalBlock().type() < mGame.activeBlock(tx).type())
        {
            prec.erase(prec.begin());
        }

        if (prec.front().originalBlock().type() == mGame.activeBlock(tx).type())
        {
            if (Game::MoveResult_Committed == move(tx, mGame, prec.front().originalBlock()))
            {
                prec.erase(prec.begin());
            }
        }
        else
        {
            mPrecalculated.open_rw(tx).clear();
            STM::set(mSyncError, true);
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
    mImpl->mWorkerPool.interruptAndClearQueue();
    mImpl->mWorker.interruptAndClearQueue();
    mImpl->mCoordinationTimer.stop();
    mImpl->mMoveTimer.stop();
    mImpl.reset();
}


void Computer::setSearchDepth(unsigned inSearchDepth)
{
    STM::set(mImpl->mNumMovesPerSecond, inSearchDepth);
}


unsigned Computer::searchDepth() const
{
    return STM::get(mImpl->mSearchDepth);
}


void Computer::setSearchWidth(unsigned inSearchWidth)
{
    STM::set(mImpl->mSearchWidth, inSearchWidth);
}


unsigned Computer::searchWidth() const
{
    return STM::get(mImpl->mSearchWidth);
}


void Computer::setWorkerCount(unsigned inWorkerCount)
{
    STM::set(mImpl->mWorkerCount, inWorkerCount);
}


unsigned Computer::workerCount() const
{
    return STM::get(mImpl->mWorkerCount);
}


void Computer::setMoveSpeed(unsigned inMovesPerSecond)
{
    STM::set(mImpl->mNumMovesPerSecond, inMovesPerSecond);
}


unsigned Computer::moveSpeed() const
{
    return STM::get(mImpl->mNumMovesPerSecond);
}


} // namespace Tetris
