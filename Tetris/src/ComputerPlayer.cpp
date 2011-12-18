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


static const unsigned cDefaultWorkerCount = 1;


struct ComputerPlayer::Impl : boost::noncopyable
{
public:
    typedef ComputerPlayer::Tweaker Tweaker;

    Impl(ComputerPlayer * inComputerPlayer,
         BlockMover * inBlockMover) :
        mComputerPlayer(inComputerPlayer),
        mTweaker(0),
        mMainWorker("ComputerPlayer: MainWorker"),
        mWorkerPool("ComputerPlayer: WorkerPool", cDefaultWorkerCount),
        mEvaluator(&MakeTetrises::Instance()),
        mBlockMover(inBlockMover),
        mSearchDepth(6),
        mSearchWidth(4),
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

    void timerEvent();

    void move();

    static void UpdateComputerBlockMoveSpeed(boost::weak_ptr<Player> weakPlayer) {
        if (PlayerPtr playerPtr = weakPlayer.lock())
        {
            ComputerPlayer & cp = dynamic_cast<ComputerPlayer&>(*playerPtr);
            cp.mImpl.lock()->updateComputerBlockMoveSpeed();
        }
    }

    void updateComputerBlockMoveSpeed();

    void startNodeCalculator();
    void onStarted();
    void onWorking();
    void onStopped();
    void onFinished();
    void onError();

    // Return a copy!
    GameState previousGameState()
    {
        return mPrecalculated.empty() ? mComputerPlayer->game()->game().lock()->gameState()
                                      : mPrecalculated.back();
    }

    Block previousActiveBlock()
    {
        return mPrecalculated.empty() ? mComputerPlayer->game()->activeBlock()
                                      : mPrecalculated.back().originalBlock();
    }

    ComputerPlayer * mComputerPlayer;
    Tweaker * mTweaker;

    Worker mMainWorker;
    WorkerPool mWorkerPool;
    std::vector<GameState> mPrecalculated;
    boost::scoped_ptr<NodeCalculator> mNodeCalculator;
    const Evaluator * mEvaluator;
    boost::scoped_ptr<BlockMover> mBlockMover;
    int mSearchDepth;
    int mSearchWidth;
    int mWorkerCount;
    bool mStop;
    bool mReset;
    bool mQuitFlag;
};


PlayerPtr ComputerPlayer::Create(const TeamName & inTeamName,
                                 const PlayerName & inPlayerName,
                                 std::size_t inRowCount,
                                 std::size_t inColumnCount)
{
    PlayerPtr result(new ComputerPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
    return result;
}


ComputerPlayer::ComputerPlayer(const TeamName & inTeamName,
                               const PlayerName & inPlayerName,
                               std::size_t inRowCount,
                               std::size_t inColumnCount) :
    Player(PlayerType_Computer, inTeamName, inPlayerName, inRowCount, inColumnCount),
    mImpl(new Impl(this, new BlockMover(*game()))),
    mTimer(new Futile::Timer(10))
{


    mTimer->start(boost::bind(&ComputerPlayer::onTimerEvent, this));
}


ComputerPlayer::~ComputerPlayer()
{
    mTimer->stop();
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mQuitFlag = true;
        impl.mBlockMover.reset();
    }
}


void ComputerPlayer::onTimerEvent()
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
        LogError(SS() << "Exception caught during ComputerPlayer timerEvent. Details: " << exc.what());
    }
}


void ComputerPlayer::setTweaker(Tweaker *inTweaker)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mTweaker = inTweaker;
    }
}


int ComputerPlayer::searchDepth() const
{
    return mImpl.lock()->mSearchDepth;
}


void ComputerPlayer::setSearchDepth(int inSearchDepth)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mSearchDepth = inSearchDepth;
    }
}


int ComputerPlayer::searchWidth() const
{
    return mImpl.lock()->mSearchWidth;
}


void ComputerPlayer::setSearchWidth(int inSearchWidth)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mSearchWidth = inSearchWidth;
    }
}


int ComputerPlayer::depth() const
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


int ComputerPlayer::moveSpeed() const
{
    return mImpl.lock()->mBlockMover->speed();
}


void ComputerPlayer::setMoveSpeed(int inMoveSpeed)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.mBlockMover->setSpeed(inMoveSpeed);
    }
}


int ComputerPlayer::workerCount() const
{
    return mImpl.lock()->mWorkerCount;
}


void ComputerPlayer::setWorkerCount(int inWorkerCount)
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


void ComputerPlayer::Impl::updateComputerBlockMoveSpeed()
{
    // Consult the Tweaker for improved settings
    if (mTweaker)
    {
        BlockMover::MoveDownBehavior moveDownBehavior = BlockMover::MoveDownBehavior_Null;
        int moveSpeed = 0;

        mEvaluator = &mTweaker->updateAIParameters(*mComputerPlayer,
                                                   mSearchDepth,
                                                   mSearchWidth,
                                                   mWorkerCount,
                                                   moveSpeed,
                                                   moveDownBehavior);

        if (mSearchDepth < 1 || mSearchDepth > 100)
        {
            throw std::runtime_error(SS() << "Invalid search depth: " << mSearchDepth);
        }

        if (mSearchWidth < 1 || mSearchWidth > 100)
        {
            throw std::runtime_error(SS() << "Invalid search width: " << mSearchWidth);
        }

        if (mWorkerCount < 1 || mWorkerCount > 128)
        {
            throw std::runtime_error(SS() << "Invalid worker count: " << mWorkerCount);
        }

        if (moveSpeed != 0)
        {
            if (moveSpeed < 0)
            {
                throw std::runtime_error(SS() << "Invalid move speed: " << moveSpeed);
            }
            mBlockMover->setSpeed(moveSpeed);
        }

        if (moveDownBehavior != BlockMover::MoveDownBehavior_Null)
        {
            mBlockMover->setMoveDownBehavior(moveDownBehavior);
        }
    }
}


bool Move(Game & ioGame, const Block & targetBlock)
{
    Block block = ioGame.activeBlock();
    Assert(block.type() == targetBlock.type());

    // Try rotation first, if it fails then skip rotation and try horizontal move
    if (block.rotation() != targetBlock.rotation())
    {
        if (ioGame.rotate())
        {
            return true;
        }
        // else: try left or right move below
    }

    if (block.column() < targetBlock.column())
    {
        if (!ioGame.move(MoveDirection_Right))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            ioGame.dropAndCommit();
            return false;
        }
        return true;
    }

    if (block.column() > targetBlock.column())
    {
        if (!ioGame.move(MoveDirection_Left))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            ioGame.dropAndCommit();
            return false;
        }
        return true;
    }

    // Horizontal position is OK.
    // Retry rotation again. If it fails here then drop the block.
    if (block.rotation() != targetBlock.rotation())
    {
        if (!ioGame.rotate())
        {
            ioGame.dropAndCommit();
            return false;
        }
        return true;
    }

    return ioGame.move(MoveDirection_Down);
}


void ComputerPlayer::Impl::move()
{
    if (mPrecalculated.empty())
    {
        return;
    }

    FUTILE_LOCK(Game & game, mComputerPlayer->game()->game())
    {
        unsigned oldId = game.gameStateId();
        Move(game, mPrecalculated.front().originalBlock());
        unsigned newId = game.gameStateId();

        Assert(oldId == newId || oldId == newId + 1);
        if (newId > oldId)
        {
            mPrecalculated.erase(mPrecalculated.begin());
        }
    }
}


void ComputerPlayer::Impl::timerEvent()
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


    move();


    // Consult the tweaker.
    PlayerPtr playerPtr = mComputerPlayer->shared_from_this();
    boost::weak_ptr<Player> weakPtr(playerPtr);
    InvokeLater(boost::bind(&ComputerPlayer::Impl::UpdateComputerBlockMoveSpeed, weakPtr));

    if (!mNodeCalculator)
    {
        startNodeCalculator();
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
            onStarted();
            return;
        }
        case NodeCalculator::Status_Working:
        {
            onWorking();
            return;
        }
        case NodeCalculator::Status_Stopped:
        {
            onStopped();
            return;
        }
        case NodeCalculator::Status_Finished:
        {
            onFinished();
            return;
        }
        case NodeCalculator::Status_Error:
        {
            onError();
            return;
        }
        default:
        {
            throw std::invalid_argument("Invalid enum value for NodeCalculator::Status.");
        }
    }
}


void ComputerPlayer::Impl::startNodeCalculator()
{
    if (mPrecalculated.size() > 8)
    {
        // We're fine for now.
        return;
    }

    if (previousGameState().isGameOver())
    {
        return;
    }


    //
    // Create the list of future blocks
    //
    const SimpleGame * simpleGame = mComputerPlayer->game();
    std::vector<Block> nextBlocks = simpleGame->getNextBlocks(mPrecalculated.size() + mSearchDepth);
    BlockTypes futureBlocks;
    for (std::size_t idx = mPrecalculated.size(); idx != nextBlocks.size(); ++idx)
    {
        futureBlocks.push_back(nextBlocks[idx].type());
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
    mNodeCalculator.reset(new NodeCalculator(previousGameState(),
                                             futureBlocks,
                                             widths,
                                             *mEvaluator,
                                             mMainWorker,
                                             mWorkerPool));

    mNodeCalculator->start();
}


void ComputerPlayer::Impl::onStarted()
{
    // Good.
}


void ComputerPlayer::Impl::onWorking()
{
    SimpleGame & simpleGame = *mComputerPlayer->game();

    FUTILE_LOCK(Game & game, simpleGame.game())
    {
        if (previousGameState().id() < game.gameState().id())
        {
            // The calculated results have become invalid. Start over.
            mReset = true;
            return;
        }

        if (mPrecalculated.empty())
        {
            mStop = true;
            return;
        }
    }

    // Keep working
}


void ComputerPlayer::Impl::onStopped()
{
    // Good. Now wait until onFinished.
}


void ComputerPlayer::Impl::onFinished()
{
    mReset = true;

    std::vector<GameState> results = mNodeCalculator->result();
    std::copy(results.begin(), results.end(), std::back_inserter(mPrecalculated));


    if (mPrecalculated.empty())
    {
        return;
    }

    for (std::size_t idx = 0; idx + 1 < mPrecalculated.size(); ++idx)
    {
        Assert(mPrecalculated[idx].id() == mPrecalculated[idx + 1].id());
    }
}


void ComputerPlayer::Impl::onError()
{
    mReset = true;
    LogError("ComputerPlayer: " + mNodeCalculator->errorMessage());
}


} // namespace Tetris
