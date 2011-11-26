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
#include <set>
#include <stdexcept>


namespace Tetris {


using namespace Futile;


struct SimpleGame::Impl : boost::enable_shared_from_this<SimpleGame::Impl>
{
    static Futile::ThreadSafe<Game> CreateGame(PlayerType inPlayerType, std::size_t inRowCount, std::size_t inColumnCount)
    {
        if (inPlayerType == PlayerType_Human)
        {
            return HumanGame::Create(inRowCount, inColumnCount);
        }
        else if (inPlayerType == PlayerType_Computer)
        {
            return ComputerGame::Create(inRowCount, inColumnCount);
        }
        throw std::logic_error("Invalid enum value for PlayerType.");
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


ThreadSafe<Game> SimpleGame::gameImpl() const
{
    return mImpl->mGame;
}


void SimpleGame::setPaused(bool inPaused)
{
    gameImpl().lock()->setPaused(inPaused);
}


bool SimpleGame::isPaused() const
{
    return gameImpl().lock()->isPaused();
}


GameStateStats SimpleGame::stats() const
{
    const Locker<Game> locker(gameImpl());
    const GameState & gameState = locker->gameState();
    return GameStateStats(gameState.numLines(),
                          gameState.numSingles(),
                          gameState.numDoubles(),
                          gameState.numTriples(),
                          gameState.numTetrises(),
                          gameState.currentHeight());
}


void SimpleGame::applyLinePenalty(int inNumberOfLinesMadeByOpponent)
{
    LogInfo(SS() << (playerType() == PlayerType_Computer ? "The computer" : "The human being") << " received " << inNumberOfLinesMadeByOpponent << " lines from his crafty opponent");
    return gameImpl().lock()->applyLinePenalty(inNumberOfLinesMadeByOpponent);
}


bool SimpleGame::isGameOver() const
{
    return gameImpl().lock()->isGameOver();
}


int SimpleGame::rowCount() const
{
    return gameImpl().lock()->rowCount();
}


int SimpleGame::columnCount() const
{
    return gameImpl().lock()->columnCount();
}


void SimpleGame::move(MoveDirection inDirection)
{
    gameImpl().lock()->move(inDirection);
}


void SimpleGame::rotate()
{
    gameImpl().lock()->rotate();
}


void SimpleGame::drop()
{
    gameImpl().lock()->dropAndCommit();
}


void SimpleGame::setStartingLevel(int inLevel)
{
    gameImpl().lock()->setStartingLevel(inLevel);
}


int SimpleGame::level() const
{
    return gameImpl().lock()->level();
}


Block SimpleGame::activeBlock() const
{
    return gameImpl().lock()->activeBlock();
}


Grid SimpleGame::gameGrid() const
{
    return gameImpl().lock()->gameGrid();
}


Block SimpleGame::getNextBlock() const
{
    std::vector<BlockType> blockTypes;
    gameImpl().lock()->getFutureBlocks(2, blockTypes);

    if (blockTypes.size() != 2)
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    return Block(blockTypes.back(), Rotation(0), Row(0), Column((columnCount() - mImpl->mCenterColumn)/2));
}


std::vector<Block> SimpleGame::getNextBlocks(std::size_t inCount) const
{
    std::vector<BlockType> blockTypes;

    gameImpl().lock()->getFutureBlocks(1 + inCount, blockTypes);

    if (blockTypes.size() != (1 + inCount))
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    std::vector<Block> result;
    for (std::vector<BlockType>::size_type idx = 1; idx < blockTypes.size(); ++idx)
    {
        result.push_back(Block(blockTypes[idx],
                               Rotation(0),
                               Row(0),
                               Column((columnCount() - mImpl->mCenterColumn)/2)));
    }
    return result;
}


} // namespace Tetris
