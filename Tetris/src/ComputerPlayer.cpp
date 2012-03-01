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
#include <iostream> // TODO: cleanup


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


namespace {


struct NonMemoryResources
{
    NonMemoryResources() :
        mMainWorker("Computer main worker"),
        mWorkerPool("Computer worker pool", cDefaultWorkerCount),
        mNodeCalculator()
    {
    }

    Worker mMainWorker;
    WorkerPool mWorkerPool;
    boost::scoped_ptr<NodeCalculator> mNodeCalculator;
};


typedef std::vector<GameState> Precalculated;
typedef std::vector<GameState> Intermediates;


struct NodeCalculatorParams
{
    NodeCalculatorParams(const GameState & inGameState,
                         const BlockTypes & inBlockTypes,
                         const Widths & inWidths,
                         unsigned inWorkerCount) :
        mGameState(inGameState),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mWorkerCount(inWorkerCount)
    {
    }

    GameState mGameState;
    BlockTypes mBlockTypes;
    Widths mWidths;
    unsigned mWorkerCount;
};


struct SyncError : public std::runtime_error
{
    SyncError() :
        std::runtime_error("SyncError")
    {
    }

    virtual ~SyncError() throw() {}
};


struct GameOver : public std::runtime_error
{
    GameOver() :
        std::runtime_error("GameOver")
    {

    }

    virtual ~GameOver() throw() {}
};


} // anonymous namespace


struct Computer::Impl : boost::noncopyable
{
    typedef std::vector<GameState> Result;

    const Evaluator * mEvaluator;
    ThreadSafe<NonMemoryResources> mNonMemoryResources;
    Game & mGame; // Game uses STM internally.
    mutable stm::shared<Precalculated> mPrecalculated;
    mutable stm::shared<Intermediates> mIntermediates;
    mutable stm::shared<int> mSearchDepth;
    mutable stm::shared<int> mSearchWidth;
    mutable stm::shared<int> mWorkerCount;

    Impl(Game & inGame) :
        mEvaluator(&MakeTetrises::Instance()),
        mNonMemoryResources(new NonMemoryResources),
        mGame(inGame),
        mPrecalculated(Precalculated()),
        mIntermediates(Intermediates()),
        mSearchDepth(8),
        mSearchWidth(5),
        mWorkerCount(cDefaultWorkerCount)
    {
    }

    ~Impl()
    {
        FUTILE_LOCK(NonMemoryResources & res, mNonMemoryResources)
        {
            if (res.mNodeCalculator)
            {
                res.mNodeCalculator->stop();
            }
            res.mWorkerPool.interruptAndClearQueue();
        }
    }

    Precalculated getPrecalculatedCopy() const
    {
        return stm::atomic<Precalculated>([&](stm::transaction & tx) { return mPrecalculated.open_r(tx); });
    }

    void checkForSyncErrors()
    {
        stm::atomic([&](stm::transaction & tx) {
            Precalculated & precalculated = mPrecalculated.open_rw(tx);
            if (!precalculated.empty())
            {
                unsigned currId = mGame.gameState(tx).id();
                while (!precalculated.empty() && precalculated.front().id() != currId + 1)
                {
                    precalculated.erase(precalculated.begin());
                    LogWarning("Sync error");
                }
            }
        });
    }

    static unsigned GetTimerIntervalMs(unsigned inNumMovesPerSecond)
    {
        return static_cast<unsigned>(0.5 + 1000.0 / static_cast<double>(inNumMovesPerSecond));
    }

    void moveTimerEvent();
    void moveTimerEventImpl(stm::transaction & tx);

    Game::MoveResult move(stm::transaction & tx, const GameState & inNextGameState);

    NodeCalculatorParams getNodeCalculatorParams();

    NodeCalculatorParams getNodeCalculatorParams(stm::transaction & tx);

    void restartNodeCalculator();

    void startNodeCalculator(NonMemoryResources & res,
                             const GameState & inGameState,
                             const std::vector<BlockType> & inBlockTypes,
                             const std::vector<int> & inWidths,
                             unsigned inWorkerCount);

    void processResults(const std::vector<GameState> & results);

    void processResults(stm::transaction & tx, const std::vector<GameState> & results);

    // Return a copy!
    GameState previousGameState()
    {
        return stm::atomic<GameState>([&](stm::transaction & tx) { return previousGameState(tx); });
    }

    const GameState & previousGameState(stm::transaction & tx)
    {
        const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
        return cPrecalculated.empty() ? mGame.gameState(tx) : cPrecalculated.back();
    }

    Block previousActiveBlock(stm::transaction & tx)
    {
        const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
        return cPrecalculated.empty() ? mGame.activeBlock(tx)
                                      : cPrecalculated.back().originalBlock();
    }
};


Computer::Computer(Game & inGame) :
    mImpl(new Impl(inGame)),
    mMoveTimer(new Futile::Timer(10))
{
    this->setMoveSpeed(50);
    mMoveTimer->start(boost::bind(&Computer::onMoveTimerEvent, this));
}


Computer::~Computer()
{
    mMoveTimer->stop();
}


void Computer::onMoveTimerEvent()
{
    try
    {
        mImpl->moveTimerEvent();
    }
    catch (const std::exception & exc)
    {
        LogError(SS() << "Exception caught during Computer timerEvent. Details: " << exc.what());
    }
}


void Computer::Impl::moveTimerEvent()
{
    checkForSyncErrors();

    enum {
        cMinimumPrecalculated = 4
    };

    ThreadSafe<bool> fReset(new bool(true));
    bool val = fReset.get();
    if (val)
    {
        std::vector<GameState> result;

        FUTILE_LOCK(NonMemoryResources & res, mNonMemoryResources)
        {
            if (!res.mNodeCalculator)
            {
                break;
            }
            res.mNodeCalculator->stop();
            result = res.mNodeCalculator->result();
        }

        stm::atomic([&](stm::transaction & tx)
        {
            Precalculated & prec = mPrecalculated.open_rw(tx);
            std::copy(result.begin(), result.end(), std::back_inserter(prec));
        });

        restartNodeCalculator();
    }

    stm::atomic([&](stm::transaction & tx)
    {
        const Precalculated & pc = mPrecalculated.open_r(tx);
        if (pc.size() < cMinimumPrecalculated)
        {
            fReset.set(true);
        }

        if (!pc.empty())
        {
            Precalculated & p = mPrecalculated.open_rw(tx);
            move(tx, p.front());
            p.erase(p.begin());
        }
    });
}


void Computer::Impl::restartNodeCalculator()
{
    NodeCalculatorParams params = getNodeCalculatorParams();
    FUTILE_LOCK(NonMemoryResources & res, mNonMemoryResources)
    {
        startNodeCalculator(res,
                            params.mGameState,
                            params.mBlockTypes,
                            params.mWidths,
                            params.mWorkerCount);
    }
}


int Computer::searchDepth() const
{
    return stm::atomic<int>([&](stm::transaction & tx){
        return mImpl->mSearchDepth.open_r(tx);
    });
}


void Computer::setSearchDepth(int inSearchDepth)
{
    stm::atomic([&](stm::transaction & tx){
        mImpl->mSearchDepth.open_rw(tx) = inSearchDepth;
    });
}


int Computer::searchWidth() const
{
    return stm::atomic<int>([&](stm::transaction & tx){
        return mImpl->mSearchWidth.open_r(tx);
    });
}


void Computer::setSearchWidth(int inSearchWidth)
{
    stm::atomic([&](stm::transaction & tx){
        mImpl->mSearchDepth.open_rw(tx) = inSearchWidth;
    });
}


int Computer::moveSpeed() const
{
    return mMoveTimer->interval();
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

    mMoveTimer->setInterval(Impl::GetTimerIntervalMs(inNumMovesPerSecond));
}


int Computer::workerCount() const
{
    return stm::atomic<int>([&](stm::transaction & tx){
        return mImpl->mWorkerCount.open_r(tx);
    });
}


void Computer::setWorkerCount(int inWorkerCount)
{
    stm::atomic([&](stm::transaction & tx){
        mImpl->mWorkerCount.open_rw(tx) = inWorkerCount;
    });
}


namespace {


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


} // anonymous namespace


Game::MoveResult Computer::Impl::move(stm::transaction & tx, const GameState & inNextGameState)
{
    unsigned oldId = mGame.gameStateId(tx);
    if (oldId + 1 != inNextGameState.id())
    {
        throw SyncError();
    }

    Game::MoveResult res = Move(tx, mGame, inNextGameState.originalBlock());

    Assert((
        (res == Game::MoveResult_Moved || res == Game::MoveResult_NotMoved) && mGame.gameStateId(tx) == oldId)
        || (res == Game::MoveResult_Commited && mGame.gameStateId(tx) == oldId + 1));

    return res;
}


NodeCalculatorParams Computer::Impl::getNodeCalculatorParams()
{
    return stm::atomic<NodeCalculatorParams>([&](stm::transaction & tx) {
        return getNodeCalculatorParams(tx);
    });
}


NodeCalculatorParams Computer::Impl::getNodeCalculatorParams(stm::transaction & tx)
{
    if (previousGameState(tx).isGameOver())
    {
        throw GameOver();
    }

    //
    // Create the list of future blocks
    //
    const Precalculated & cPrecalculated = mPrecalculated.open_r(tx);
    std::vector<BlockType> nextBlocks = mGame.getFutureBlocks(tx, cPrecalculated.size() + mSearchDepth.open_r(tx) + 1);
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
        widths.push_back(mSearchWidth.open_r(tx));
    }

    return NodeCalculatorParams(previousGameState(tx), futureBlocks, widths, cDefaultWorkerCount);
}


void Computer::Impl::startNodeCalculator(NonMemoryResources & res,
                                         const GameState & inGameState,
                                         const std::vector<BlockType> & inBlockTypes,
                                         const std::vector<int> & inWidths,
                                         unsigned inWorkerCount)
{
    LogInfo(SS()  << "Actually starting the node calculator.");
    res.mWorkerPool.resize(inWorkerCount);
    res.mNodeCalculator.reset(new NodeCalculator(inGameState,
                                                 inBlockTypes,
                                                 inWidths,
                                                 *mEvaluator,
                                                 res.mMainWorker,
                                                 res.mWorkerPool));
    res.mNodeCalculator->start([this](const std::vector<GameState> & result) {
        stm::atomic([&](stm::transaction & tx) {
            mIntermediates.open_rw(tx) = result;
        });
    });
}


void Computer::Impl::processResults(const std::vector<GameState> & results)
{
    stm::atomic([&](stm::transaction & tx) { return processResults(tx, results); });
}


void Computer::Impl::processResults(stm::transaction & tx, const std::vector<GameState> & results)
{
    Precalculated & precalculated = mPrecalculated.open_rw(tx);
    Assert(precalculated.empty() || precalculated.front().id() == mGame.gameStateId(tx) + 1);
    std::stringstream ss;
    ss << "Precalculated " << precalculated.size() << " + " << results.size() << " items: ";
    for (std::size_t idx = 0; idx < results.size(); ++idx)
    {
        const GameState & gameState = results[idx];
        Assert(precalculated.empty() || gameState.id() == precalculated.back().id() + 1);
        precalculated.push_back(gameState);

        if (idx != 0)
        {
            ss << ", ";
        }
        ss << gameState.id();
    }
    LogInfo(ss.str());
}


} // namespace Tetris
