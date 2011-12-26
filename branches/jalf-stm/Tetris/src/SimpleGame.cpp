#include "Tetris/ComputerPlayer.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/Gravity.h"
#include "Futile/Logging.h"
#include "Futile/Threading.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/MainThread.h"
#include "Futile/MakeString.h"
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/weak_ptr.hpp>
#include <stdexcept>


namespace Tetris {


using namespace Futile;


struct SimpleGame::Impl : boost::enable_shared_from_this<SimpleGame::Impl>
{
    static Futile::ThreadSafe<Game> CreateGame(PlayerType /*inPlayerType*/, std::size_t inRowCount, std::size_t inColumnCount)
    {
        Futile::ThreadSafe<Game> result(new Game(inRowCount, inColumnCount));
        return result;
    }

    Impl(SimpleGame * inSimpleGame,
         PlayerType inPlayerType,
         std::size_t inRowCount,
         std::size_t inColumnCount) :
        mSimpleGame(inSimpleGame),
        mGame(CreateGame(inPlayerType, inRowCount, inColumnCount)),
        mPlayerType(inPlayerType),
        mGravity(new Gravity(mGame)),
        mCenterColumn(static_cast<std::size_t>(0.5 + inColumnCount / 2.0))
    {
        FUTILE_LOCK(Game & game, mGame)
        {
            game.GameStateChanged.connect(boost::bind(&Impl::onGameStateChanged, this));
            game.LinesCleared.connect(boost::bind(&Impl::onLinesCleared, this, _1));
        }
    }

    ~Impl()
    {
    }

    virtual void onGameStateChanged()
    {
        // This method is triggered in a worker thread. Dispatch to main thread.
        boost::weak_ptr<Impl> weakSelf(shared_from_this());
        InvokeLater(boost::bind(&Impl::OnGameStateChangedLater, weakSelf));

    }

    static void OnGameStateChangedLater(boost::weak_ptr<Impl> inWeakImpl)
    {
        if (boost::shared_ptr<Impl> impl = inWeakImpl.lock())
        {
            impl->handleGameStateChanged();
        }
    }

    typedef std::set<SimpleGame::EventHandler*> EventHandlers;

    void handleGameStateChanged()
    {
        EventHandlers::iterator it = mEventHandlers.begin(), end = mEventHandlers.end();
        for (; it != end; ++it)
        {
            SimpleGame::EventHandler * eventHandler(*it);
            eventHandler->onGameStateChanged(mSimpleGame);
        }
    }

    virtual void onLinesCleared(std::size_t inLineCount)
    {
        // This method is triggered in a worker thread. Dispatch to main thread.
        boost::weak_ptr<Impl> weakSelf(shared_from_this());
        InvokeLater(boost::bind(&Impl::OnLinesClearedLater, weakSelf, inLineCount));
    }

    static void OnLinesClearedLater(boost::weak_ptr<Impl> inWeakImpl, std::size_t inLineCount)
    {
        if (boost::shared_ptr<Impl> impl = inWeakImpl.lock())
        {
            impl->handleLinesCleared(inLineCount);
        }
    }

    void handleLinesCleared(std::size_t inLineCount)
    {
        EventHandlers::iterator it = mEventHandlers.begin(), end = mEventHandlers.end();
        for (; it != end; ++it)
        {
            SimpleGame::EventHandler * eventHandler(*it);
            eventHandler->onLinesCleared(mSimpleGame, inLineCount);
        }
    }

    SimpleGame * mSimpleGame;
    std::string mName;
    ThreadSafe<Game> mGame;
    PlayerType mPlayerType;
    boost::scoped_ptr<Gravity> mGravity;
    std::size_t mCenterColumn;
    typedef boost::signals2::scoped_connection ScopedConnection;
    ScopedConnection mGameStateChanged;
    ScopedConnection mLinesCleared;
    EventHandlers mEventHandlers;
};


SimpleGame::SimpleGame(PlayerType inPlayerType, std::size_t inRowCount, std::size_t inColumnCount) :
    mImpl(new Impl(this, inPlayerType, inRowCount, inColumnCount))
{
}


SimpleGame::~SimpleGame()
{
    mImpl.reset();
}


ThreadSafe<Game> SimpleGame::game()
{
    return mImpl->mGame;
}


/*GameState & SimpleGame::gameState()
{
    return mImpl->mGame.lock()->gameState();
}*/


bool SimpleGame::checkPositionValid(const Block & inBlock) const
{
    return mImpl->mGame.lock()->checkPositionValid(inBlock);
}


PlayerType SimpleGame::playerType() const
{
    return mImpl->mPlayerType;
}


void SimpleGame::registerEventHandler(EventHandler * inEventHandler)
{
    mImpl->mEventHandlers.insert(inEventHandler);
}


void SimpleGame::unregisterEventHandler(EventHandler * inEventHandler)
{
    mImpl->mEventHandlers.erase(inEventHandler);
}


void SimpleGame::setPaused(bool inPaused)
{
    mImpl->mGame.lock()->setPaused(inPaused);
}


bool SimpleGame::isPaused() const
{
    return mImpl->mGame.lock()->isPaused();
}


GameStateStats SimpleGame::stats() const
{
    const Locker<Game> locker(mImpl->mGame);
    const GameState & gameState = locker->gameState();
    return GameStateStats(gameState.numLines(),
                          gameState.numSingles(),
                          gameState.numDoubles(),
                          gameState.numTriples(),
                          gameState.numTetrises());
}


void SimpleGame::applyLinePenalty(int inNumberOfLinesMadeByOpponent)
{
    LogInfo(SS() << (playerType() == PlayerType_Computer ? "The computer" : "The human being") << " received " << inNumberOfLinesMadeByOpponent << " lines from his crafty opponent");
    return mImpl->mGame.lock()->applyLinePenalty(inNumberOfLinesMadeByOpponent);
}


bool SimpleGame::isGameOver() const
{
    return mImpl->mGame.lock()->isGameOver();
}


int SimpleGame::rowCount() const
{
    return mImpl->mGame.lock()->rowCount();
}


int SimpleGame::columnCount() const
{
    return mImpl->mGame.lock()->columnCount();
}


bool SimpleGame::move(Direction inDirection)
{
    return mImpl->mGame.lock()->move(inDirection);
}


bool SimpleGame::rotate()
{
    return mImpl->mGame.lock()->rotate();
}


void SimpleGame::drop()
{
    mImpl->mGame.lock()->dropAndCommit();
}


void SimpleGame::setStartingLevel(int inLevel)
{
    mImpl->mGame.lock()->setStartingLevel(inLevel);
}


int SimpleGame::level() const
{
    return mImpl->mGame.lock()->level();
}


Block SimpleGame::activeBlock() const
{
    return stm::atomic<Block>([&](stm::transaction & tx) {
        Locker<Game> locker(mImpl->mGame);
        const Game & game = *locker.get();
        return game.activeBlock().open_r(tx);
    });
}


Grid SimpleGame::gameGrid() const
{
    return mImpl->mGame.lock()->gameGrid();
}


Block SimpleGame::getNextBlock() const
{
    std::vector<BlockType> blockTypes;
    FUTILE_LOCK(Game & game, mImpl->mGame)
    {
        blockTypes = game.getFutureBlocks(2);
    }

    if (blockTypes.size() != 2)
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    return Block(blockTypes.back(), Rotation(0), Row(0), Column((columnCount() - mImpl->mCenterColumn)/2));
}


std::vector<Block> SimpleGame::getNextBlocks(std::size_t inCount) const
{
    std::vector<BlockType> blockTypes;
    FUTILE_LOCK(Game & game, mImpl->mGame)
    {
        blockTypes = game.getFutureBlocks(inCount);
        Assert(blockTypes.size() == inCount);
    }

    std::vector<Block> result;
    for (std::vector<BlockType>::size_type idx = 0; idx != blockTypes.size(); ++idx)
    {
        result.push_back(Block(blockTypes[idx],
                               Rotation(0),
                               Row(0),
                               Column((columnCount() - mImpl->mCenterColumn)/2)));
    }
    Assert(result.size() == inCount);
    return result;
}


} // namespace Tetris
