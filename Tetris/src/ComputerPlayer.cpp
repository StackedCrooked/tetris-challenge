#include "Poco/Foundation.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/Game.h"
#include "Tetris/Gravity.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Futile/MainThread.h"
#include "Futile/MakeString.h"
#include "Futile/WorkerPool.h"
#include "Futile/Worker.h"
#include "Futile/Threading.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include "Poco/Environment.h"
#include "Futile/Timer.h"
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/weak_ptr.hpp>


namespace Tetris {


using namespace Futile;


ComputerPlayer::ComputerPlayer(const TeamName & inTeamName,
                               const PlayerName & inPlayerName,
                               std::size_t inRowCount,
                               std::size_t inColumnCount) :
    Player(PlayerType_Computer, inTeamName, inPlayerName, inRowCount, inColumnCount),
    Computer(Player::game().game())
{
}


static const unsigned cDefaultWorkerCount = 8;


struct Computer::Impl : boost::noncopyable
{
public:
    Impl(Computer * inComputer, Game & inGame) :
        mComputer(inComputer),
        mGame(inGame),
        mMainWorker("Computer: MainWorker"),
        mWorkerPool("Computer: WorkerPool", cDefaultWorkerCount),
        mPrecalculated(Precalculated()),
        mNodeCalculator(),
        mEvaluator(&Balanced::Instance()),
        mSearchDepth(8),
        mSearchWidth(5),
        mWorkerCount(cDefaultWorkerCount),
        mStop(false),
        mReset(false),
        mQuitFlag(false)
    {
    }

    ~Impl()
    {
        if (mNodeCalculator)
        {
            mNodeCalculator->stop();
        }
        mWorkerPool.interruptAndClearQueue();
    }

    static unsigned GetTimerIntervalMs(unsigned inNumMovesPerSecond)
    {
        return static_cast<unsigned>(0.5 + 1000.0 / static_cast<double>(inNumMovesPerSecond));
    }

    void timerEvent();
    void timerEventImpl(stm::transaction & tx);

    void move(stm::transaction & tx);

    static void UpdateComputerBlockMoveSpeed(boost::weak_ptr<Player> weakPlayer) {
        if (PlayerPtr playerPtr = weakPlayer.lock())
        {
            Computer & cp = dynamic_cast<Computer&>(*playerPtr);
            cp.mImpl.lock()->updateComputerBlockMoveSpeed();
        }
    }

    void updateComputerBlockMoveSpeed();

    void startNodeCalculator(stm::transaction & tx);
    void onStarted(stm::transaction & tx);
    void onWorking(stm::transaction & tx);
    void onStopped(stm::transaction & tx);
    void onFinished(stm::transaction & tx);
    void onError(stm::transaction & tx);

    // Return a copy!
    inline GameState previousGameState(stm::transaction & tx)
    {
        const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
        return cPrecalculated.empty() ? mGame.gameState(tx) : cPrecalculated.back();
    }

    inline Block previousActiveBlock(stm::transaction & tx)
    {
        const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
        return cPrecalculated.empty() ? mGame.activeBlock(tx)
                                      : cPrecalculated.back().originalBlock();
    }

    Computer * mComputer;
    Game & mGame;

    Worker mMainWorker;
    WorkerPool mWorkerPool;
    typedef std::vector<GameState> Precalculated;
    mutable stm::shared<Precalculated> mPrecalculated;
    boost::scoped_ptr<NodeCalculator> mNodeCalculator;
    const Evaluator * mEvaluator;
    int mSearchDepth;
    int mSearchWidth;
    int mWorkerCount;
    bool mStop;
    bool mReset;
    bool mQuitFlag;
};


Computer::Computer(Game & inGame) :
    mImpl(new Impl(this, inGame)),
    mTimer(new Futile::Timer(100))
{
  mTimer->start(boost::bind(&Computer::onTimerEvent, this));
}


Computer::~Computer()
{
    mTimer->stop();
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mQuitFlag = true;
    }
}


void Computer::onTimerEvent()
{
    try
    {
        FUTILE_LOCK(Impl & impl, mImpl)
        {
            if (!impl.mQuitFlag)
            {
                impl.timerEvent();
            }
        }
    }
    catch (const std::exception & exc)
    {
        LogError(SS() << "Exception caught during Computer timerEvent. Details: " << exc.what());
    }
}


int Computer::searchDepth() const
{
    return mImpl.lock()->mSearchDepth;
}


void Computer::setSearchDepth(int inSearchDepth)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mSearchDepth = inSearchDepth;
    }
}


int Computer::searchWidth() const
{
    return mImpl.lock()->mSearchWidth;
}


void Computer::setSearchWidth(int inSearchWidth)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mSearchWidth = inSearchWidth;
    }
}


int Computer::depth() const
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        if (impl.mNodeCalculator)
        {
            return impl.mNodeCalculator->getCurrentSearchDepth();
        }
    }
    return 0;
}


int Computer::moveSpeed() const
{
    return mTimer->interval();
}


void Computer::setMoveSpeed(int inNumMovesPerSecond)
{
    if (inNumMovesPerSecond > 1000)
    {
        inNumMovesPerSecond = 1000;
    }

    if (inNumMovesPerSecond < 1)
    {
        inNumMovesPerSecond = 1;
    }

    mTimer->setInterval(Impl::GetTimerIntervalMs(inNumMovesPerSecond));
}


int Computer::workerCount() const
{
    return mImpl.lock()->mWorkerCount;
}


void Computer::setWorkerCount(int inWorkerCount)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        if (inWorkerCount == 0)
        {
            impl.mWorkerCount = cDefaultWorkerCount;
        }
        else
        {
            impl.mWorkerCount = inWorkerCount;
        }
    }
}


bool Move(stm::transaction & tx, Game & ioGame, const Block & targetBlock)
{
    const Block & block = ioGame.activeBlock(tx);
    Assert(block.type() == targetBlock.type());

    // Try rotation first, if it fails then skip rotation and try horizontal move
    if (block.rotation() != targetBlock.rotation())
    {
        if (ioGame.rotate(tx))
        {
            return true;
        }
        // else: try left or right move below
    }

    if (block.column() < targetBlock.column())
    {
        if (!ioGame.move(tx, MoveDirection_Right))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            ioGame.dropAndCommit(tx);
            return false;
        }
        return true;
    }

    if (block.column() > targetBlock.column())
    {
        if (!ioGame.move(tx, MoveDirection_Left))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            ioGame.dropAndCommit(tx);
            return false;
        }
        return true;
    }

    // Horizontal position is OK.
    // Retry rotation again. If it fails here then drop the block.
    if (block.rotation() != targetBlock.rotation())
    {
        if (!ioGame.rotate(tx))
        {
            ioGame.dropAndCommit(tx);
            return false;
        }
        return true;
    }

    ioGame.move(tx, MoveDirection_Down);
    return false;
}


void Computer::Impl::move(stm::transaction & tx)
{
    const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
    if (cPrecalculated.empty())
    {
        return;
    }

    unsigned oldId = mGame.gameStateId(tx);
    unsigned predictedId = cPrecalculated.front().id();
    if (oldId >= predictedId)
    {
        // Precalculated blocks have been invalidated.
        LogInfo("Precalculated invalidated.");
        mPrecalculated.open_rw(tx).clear();
        return;
    }

    Assert(predictedId == oldId + 1); // check sync
    const Block & activeBlock = mGame.activeBlock(tx);
    const Block & originalBlock = cPrecalculated.front().originalBlock();
    Assert(activeBlock.type() == originalBlock.type());
    (void)activeBlock;
    (void)originalBlock;

    Move(tx, mGame, cPrecalculated.front().originalBlock());
    unsigned newId = mGame.gameStateId(tx);
    Assert(oldId == newId || oldId + 1 == newId);
    if (newId > oldId)
    {
        Precalculated & precalculated = mPrecalculated.open_rw(tx);
        precalculated.erase(precalculated.begin());
    }
}


void Computer::Impl::timerEvent()
{
    stm::atomic([&](stm::transaction & tx) {
        timerEventImpl(tx);
    });
}


void Computer::Impl::timerEventImpl(stm::transaction & tx)
{
    if (mReset)
    {
        mReset = false;
        mNodeCalculator.reset();
        return;
    }

    if (mStop)
    {
        mStop = false;
        mNodeCalculator->stop();
        return;
    }

    move(tx);

    if (!mNodeCalculator)
    {
        startNodeCalculator(tx);
        return;
    }

    switch (mNodeCalculator->status())
    {
        case NodeCalculator::Status_Nil:
        {
            throw std::logic_error("Status should be Status_Started or higher.");
        }
        case NodeCalculator::Status_Started:
        {
            onStarted(tx);
            return;
        }
        case NodeCalculator::Status_Working:
        {
            onWorking(tx);
            return;
        }
        case NodeCalculator::Status_Stopped:
        {
            onStopped(tx);
            return;
        }
        case NodeCalculator::Status_Finished:
        {
            onFinished(tx);
            return;
        }
        case NodeCalculator::Status_Error:
        {
            onError(tx);
            return;
        }
        default:
        {
            throw std::invalid_argument("Invalid enum value for NodeCalculator::Status.");
        }
    }
}


void Computer::Impl::startNodeCalculator(stm::transaction & tx)
{
    const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
    if (cPrecalculated.size() > 8)
    {
        // We're fine for now.
        return;
    }

    if (previousGameState(tx).isGameOver())
    {
        return;
    }


    //
    // Create the list of future blocks
    //
    std::vector<BlockType> nextBlocks = mGame.getFutureBlocks(tx, cPrecalculated.size() + mSearchDepth);
    Assert(nextBlocks.size() == cPrecalculated.size() + mSearchDepth);
    BlockTypes futureBlocks;
    for (std::size_t idx = cPrecalculated.size(); idx < nextBlocks.size(); ++idx)
    {
        futureBlocks.push_back(nextBlocks[idx]);
    }


    //
    // Fill list of search widths (using the same width for each level).
    //
    std::vector<int> widths;
    for (std::size_t idx = 0; idx != futureBlocks.size(); ++idx)
    {
        widths.push_back(mSearchWidth);
    }


    //
    // Create and start the NodeCalculator object.
    //
    Assert(mWorkerPool.getActiveWorkerCount() == 0);
    if (mWorkerCount == 0)
    {
        mWorkerCount = cDefaultWorkerCount;
    }
    mWorkerPool.resize(mWorkerCount);
    mNodeCalculator.reset(new NodeCalculator(previousGameState(tx),
                                             futureBlocks,
                                             widths,
                                             *mEvaluator,
                                             mMainWorker,
                                             mWorkerPool));
    mNodeCalculator->start();
}


void Computer::Impl::onStarted(stm::transaction &)
{
    // Good.
}


void Computer::Impl::onWorking(stm::transaction & tx)
{
    if (previousGameState(tx).id() < mGame.gameState(tx).id())
    {
        // The calculated results have become invalid. Start over.
        mReset = true;
        return;
    }

    const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
    if (cPrecalculated.empty())
    {
        mStop = true;
        return;
    }

    // Keep working
}


void Computer::Impl::onStopped(stm::transaction &)
{
    // Good. Now wait until onFinished.
}


void Computer::Impl::onFinished(stm::transaction & tx)
{
    mReset = true;

    Precalculated & precalculated = mPrecalculated.open_rw(tx);
    std::vector<GameState> results = mNodeCalculator->result();
    std::copy(results.begin(), results.end(), std::back_inserter(precalculated));


    if (precalculated.empty())
    {
        return;
    }

    for (std::size_t idx = 0; idx + 1 < precalculated.size(); ++idx)
    {
        Assert(precalculated[idx].id() + 1 == precalculated[idx + 1].id());
    }
}


void Computer::Impl::onError(stm::transaction & )
{
    mReset = true;
    LogError("Computer: " + mNodeCalculator->errorMessage());
}


} // namespace Tetris
