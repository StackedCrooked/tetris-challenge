#include "Futile/AutoPtrSupport.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/Gravity.h"
#include "Futile/Logging.h"
#include "Futile/MainThread.h"
#include "Futile/MakeString.h"
#include "Tetris/SimpleGame.h"
#include "Futile/Threading.h"
#include "Futile/Timer.h"
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/weak_ptr.hpp>
#include <stdexcept>


namespace Tetris {


using namespace Futile;


struct SimpleGame::Impl : boost::enable_shared_from_this<SimpleGame::Impl>
{
    Impl(SimpleGame * inSimpleGame,
         PlayerType inPlayerType,
         std::size_t inRowCount,
         std::size_t inColumnCount) :
        mSimpleGame(inSimpleGame),
        mGame(inRowCount, inColumnCount),
        mPlayerType(inPlayerType),
        mGravity(new Gravity(mGame)),
        mCenterColumn(static_cast<std::size_t>(0.5 + inColumnCount / 2.0)),
        mTimer(10)
    {
        mTimer.start([&](){ this->mGame.GameStateChanged(); });
        mGameStateChanged = mGame.GameStateChanged.connect(boost::bind(&Impl::onGameStateChanged, this));
        mLinesCleared = mGame.LinesCleared.connect(boost::bind(&Impl::onLinesCleared, this, _1));
    }

    ~Impl()
    {
        mTimer.stop();
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
            impl->mSimpleGame->Changed(*impl->mSimpleGame);
        }
    }

    virtual void onLinesCleared(unsigned inLineCount)
    {
        // This method is triggered in a worker thread. Dispatch to main thread.
        boost::weak_ptr<Impl> weakSelf(shared_from_this());
        InvokeLater(boost::bind(&Impl::OnLinesClearedLater, weakSelf, inLineCount));
    }

    static void OnLinesClearedLater(boost::weak_ptr<Impl> inWeakImpl, unsigned inLineCount)
    {
        if (boost::shared_ptr<Impl> impl = inWeakImpl.lock())
        {
            impl->mSimpleGame->LinesCleared(*impl->mSimpleGame, inLineCount);
        }
    }

    SimpleGame * mSimpleGame;
    std::string mName;
    Game mGame;
    PlayerType mPlayerType;
    boost::scoped_ptr<Gravity> mGravity;
    std::size_t mCenterColumn;
    typedef boost::signals2::scoped_connection ScopedConnection;
    ScopedConnection mGameStateChanged;
    ScopedConnection mLinesCleared;
    Futile::Timer mTimer;
};


SimpleGame::SimpleGame(PlayerType inPlayerType, std::size_t inRowCount, std::size_t inColumnCount) :
    mImpl(new Impl(this, inPlayerType, inRowCount, inColumnCount))
{
}


SimpleGame::~SimpleGame()
{
    mImpl.reset();
}


Game & SimpleGame::game()
{
    return mImpl->mGame;
}


const Game & SimpleGame::game() const
{
    return mImpl->mGame;
}


bool SimpleGame::checkPositionValid(const Block & inBlock) const
{
    return game().checkPositionValid(inBlock);
}


PlayerType SimpleGame::playerType() const
{
    return mImpl->mPlayerType;
}


void SimpleGame::setPaused(bool inPaused)
{    
    return game().setPaused(inPaused);
}


bool SimpleGame::isPaused() const
{
    return game().isPaused();
}


GameStateStats SimpleGame::stats() const
{
    return mImpl->mGame.stats();
}


void SimpleGame::applyLinePenalty(int inNumberOfLinesMadeByOpponent)
{
    LogInfo(SS() << (playerType() == PlayerType_Computer ? "The computer" : "The human being") << " received " << inNumberOfLinesMadeByOpponent << " lines from his crafty opponent");

    stm::atomic([&](stm::transaction & tx) {
        mImpl->mGame.applyLinePenalty(tx, inNumberOfLinesMadeByOpponent);
    });
}


bool SimpleGame::isGameOver() const
{
    return stm::atomic<bool>([&](stm::transaction & tx) {
        return mImpl->mGame.isGameOver(tx);
    });
}


int SimpleGame::rowCount() const
{
    return stm::atomic<int>([&](stm::transaction & tx) {
        return mImpl->mGame.rowCount(tx);
    });
}


int SimpleGame::columnCount() const
{
    return stm::atomic<int>([&](stm::transaction & tx) {
        return mImpl->mGame.columnCount(tx);
    });
}


bool SimpleGame::move(Direction inDirection)
{
    Game::MoveResult result = stm::atomic<Game::MoveResult>([&](stm::transaction & tx) {
        return mImpl->mGame.move(tx, inDirection);
    });

    if (result != Game::MoveResult_NotMoved) {
        mImpl->mGame.GameStateChanged();
    }

    return result;
}


bool SimpleGame::rotate()
{
    return stm::atomic<bool>([&](stm::transaction & tx) {
        return mImpl->mGame.rotate(tx);
    });
}


void SimpleGame::drop()
{
    stm::atomic([&](stm::transaction & tx) {
        mImpl->mGame.dropAndCommit(tx);
    });
}


void SimpleGame::setStartingLevel(int inLevel)
{
    stm::atomic([&](stm::transaction & tx) {
        mImpl->mGame.setStartingLevel(tx, inLevel);
    });
}


int SimpleGame::level() const
{
    return stm::atomic<int>([&](stm::transaction & tx) {
        return mImpl->mGame.level(tx);
    });
}


Block SimpleGame::activeBlock() const
{
    return stm::atomic<Block>([&](stm::transaction & tx) {
        return mImpl->mGame.activeBlock(tx);
    });
}


Grid SimpleGame::gameGrid() const
{
    return mImpl->mGame.grid();
}


Block SimpleGame::getNextBlock() const
{
    return stm::atomic<Block>([&](stm::transaction & tx) {
        std::vector<BlockType> blockTypes = mImpl->mGame.getFutureBlocks(tx, 2);
        if (blockTypes.size() != 2)
        {
            throw std::logic_error("Failed to get the next block from the factory.");
        }

        return Block(blockTypes.back(), Rotation(0), Row(0), Column((columnCount() - mImpl->mCenterColumn)/2));
    });
}


std::vector<Block> SimpleGame::getNextBlocks(std::size_t inCount) const
{
    return stm::atomic< std::vector<Block> >([&](stm::transaction & tx) {
        std::vector<BlockType> blockTypes;
        blockTypes = mImpl->mGame.getFutureBlocks(tx, inCount);
        Assert(blockTypes.size() == inCount);

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
    });
}


} // namespace Tetris
