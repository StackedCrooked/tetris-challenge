#include "Tetris/Computer.h"
#include "Tetris/AISupport.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/GameState.h"
#include "Tetris/NodeCalculator.h"
#include "Futile/Assert.h"
#include "Futile/PrettyPrint.h"
#include "Futile/Logging.h"
#include "Futile/STMSupport.h"
#include "Futile/Timer.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include <iostream>
#include <vector>


namespace Tetris {


using namespace Futile;


struct Computer::Impl : boost::noncopyable
{
    static const int cDefaultNumMovesPerSecond = 50;
    static const int cDefaultSearchDepth = 8;
    static const int cDefaultSearchWidth = 2;
    static const int cDefaultWorkerCount = 1;
    static const int cSurvivalModeTreshold = 18;

    Impl(Game & inGame) :
        mGame(inGame),
        mPrecalculated(Precalculated()),
        mNumMovesPerSecond(cDefaultNumMovesPerSecond),
        mSearchDepth(cDefaultSearchDepth),
        mSearchWidth(cDefaultSearchWidth),
        mWorkerCount(cDefaultWorkerCount),
        mSyncError(false),
        mEvaluator(&MakeTetrises::Instance()),
        mWorker("Computer"),
        mWorkerPool("Computer", STM::get(mWorkerCount)),
        mMoveTimer(intervalMs(cDefaultNumMovesPerSecond)),
        mCoordinationTimer(50)
    {
    }

    ~Impl()
    {
    }

    static UInt64 intervalMs(int inFrequency)
    {
        return UInt64(0.5 + (1000.0 / inFrequency));
    }

    void move();

    Game::MoveResult move(stm::transaction & tx, Game & ioGame, const Block & targetBlock, const Evaluator & inEvaluator);

    void coordinate();

    typedef GameStateList Precalculated;
    typedef GameStateList Preliminaries;

    Game & mGame;
    mutable stm::shared<Precalculated> mPrecalculated;
    mutable stm::shared<unsigned> mNumMovesPerSecond;
    mutable stm::shared<unsigned> mSearchDepth;
    mutable stm::shared<unsigned> mSearchWidth;
    mutable stm::shared<unsigned> mWorkerCount;
    mutable stm::shared<bool> mSyncError;
    mutable stm::shared<const Evaluator*> mEvaluator;
    Worker mWorker;
    WorkerPool mWorkerPool;
    boost::shared_ptr<NodeCalculator> mNodeCalculator;
    Futile::Timer mMoveTimer;
    Futile::Timer mCoordinationTimer;
};


void Computer::Impl::coordinate()
{
    if (mGame.isGameOver())
    {
        if (mNodeCalculator)
        {
            mNodeCalculator->stop();
        }
        return;
    }

    if (STM::get(mSyncError))
    {
        LogDebug(SS() << "Sync error. Reset.");
        mNodeCalculator.reset();
        STM::set(mSyncError, false);
    }

    const unsigned cMinimumReserve = 0; // NOTE: Sync error will occur if cMinimumReserve > 0.
    if (STM::get(mPrecalculated).size() > cMinimumReserve)
    {
        return;
    }

    if (mNodeCalculator && mNodeCalculator->status() >= NodeCalculator::Status_Working)
    {
        if (mNodeCalculator->progress().current() == 0)
        {
            return;
        }
        mNodeCalculator->stop();
    }


    if (mNodeCalculator)
    {

        // If make-tetrises mode and danger then remove all precalculated results and restart in survival mode.
        std::pair<bool, int> survival_and_firstOccupiedRow = stm::atomic<std::pair<bool, int>>([&](stm::transaction & tx) {
            return std::make_pair(NULL != dynamic_cast<const Survival*>(mEvaluator.open_r(tx)),
                                  mGame.firstOccupiedRow(tx));
        });

        if (!survival_and_firstOccupiedRow.first && survival_and_firstOccupiedRow.second < cSurvivalModeTreshold)
        {
            LogDebug(SS() << "Switch to Survival!");
            mNodeCalculator.reset();
            stm::atomic([&](stm::transaction & tx) { mPrecalculated.open_rw(tx).clear(); });
            return;
        }
    }


    Preliminaries preliminaries = mNodeCalculator ? Preliminaries(mNodeCalculator->results()) : Preliminaries();
    if (mNodeCalculator && preliminaries.empty())
    {
        return;
    }

    BlockTypes blockTypes;
    boost::scoped_ptr<GameState> lastGameState;
    unsigned workerCount = mWorkerPool.size();
    const Evaluator * ev;

    stm::atomic([&](stm::transaction & tx)
    {
        ev = mEvaluator.open_r(tx);
        workerCount = mWorkerCount.open_r(tx);
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
                if (prelim.id() <= mGame.gameStateId())
                {
                    continue;
                }


                if (prelim.id() != mGame.gameStateId() + 1)
                {
                    LogError("A not-yet-understood sync error has occured.");
                    precalculated.clear();
                    break;
                }
            }
            precalculated.push_back(prelim);
        }

        preliminaries.clear();

        blockTypes = mGame.getFutureBlocks(precalculated.size() + ev->recommendedSearchDepth());
        blockTypes.erase(blockTypes.begin(), blockTypes.begin() + precalculated.size());
        lastGameState.reset(new GameState(precalculated.empty() ? mGame.gameState(tx) : precalculated.back()));
    });

    if (!blockTypes.empty() && !lastGameState->isGameOver())
    {
        const GameState & gs = *lastGameState;
        if (gs.firstOccupiedRow() < cSurvivalModeTreshold)
        {
            ev = &Survival::Instance();
        }
        else
        {
            ev = &MakeTetrises::Instance();
        }

        blockTypes.resize(std::min(int(blockTypes.size()), ev->recommendedSearchDepth()));
        Widths widths(blockTypes.size(), ev->recommendedSearchWidth());
        Assert(widths.size() == blockTypes.size());

        mWorkerPool.resize(workerCount);

        STM::set(mEvaluator, ev);
        mNodeCalculator.reset(new NodeCalculator(*lastGameState,
                                                 blockTypes,
                                                 widths,
                                                 *ev,
                                                 mWorker,
                                                 mWorkerPool));
        mNodeCalculator->start();
    }
}


Game::MoveResult Computer::Impl::move(stm::transaction & tx, Game & ioGame, const Block & targetBlock, const Evaluator & inEvaluator)
{
    Block block = ioGame.activeBlock();
    Assert(block.type() == targetBlock.type());

    // Try rotation first, if it fails then skip rotation and try horizontal move
    if (block.rotation() != targetBlock.rotation())
    {
        if (ioGame.rotate() == Game::MoveResult_Moved)
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
            ioGame.dropAndCommit();
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
            ioGame.dropAndCommit();
            STM::set(mSyncError, true);
            return Game::MoveResult_Committed;
        }
    }

    // Horizontal position is OK.
    // Retry rotation again. If it fails here then drop the block.
    if (block.rotation() != targetBlock.rotation())
    {
        if (ioGame.rotate() == Game::MoveResult_Moved)
        {
            return Game::MoveResult_Moved;
        }
        else
        {
            ioGame.dropAndCommit();
            STM::set(mSyncError, true);
            return Game::MoveResult_Committed;
        }
    }

    if (inEvaluator.moveDownBehavior() == MoveDownBehavior_DropDown)
    {
        ioGame.dropAndCommit();
        return Game::MoveResult_Committed;
    }
    else if (inEvaluator.moveDownBehavior() == MoveDownBehavior_MoveDown)
    {
        return ioGame.move(tx, MoveDirection_Down);
    }
    else
    {
        return Game::MoveResult_NotMoved;
    }
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

        auto idPrec = prec.front().id();
        auto idCurr = mGame.gameStateId();

        if (idPrec != idCurr + 1)
        {
            Assert(idPrec == idCurr);
            prec.pop_front();
        }

        if (prec.empty())
        {
            return;
        }

        // This may happen if a block was dropped after starting the node calculator
        auto t1 = prec.front().originalBlock().type();
        auto t2 = mGame.activeBlock().type();
        if (t1 != t2)
        {
            LogDebug(SS() << "Clear all precalculated.");
            prec.clear();
            return;
        }

        auto oldId = mGame.gameStateId();
        move(tx, mGame, prec.front().originalBlock(), *mEvaluator.open_r(tx));
        auto newId = mGame.gameStateId();
        Assert(oldId == newId || oldId + 1 == newId);
        if (oldId + 1 == newId)
        {
            prec.pop_front();
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
    STM::set(mImpl->mSearchDepth, inSearchDepth);
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
