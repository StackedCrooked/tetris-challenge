#include "Tetris/Config.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Game.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameImpl.h"
#include "Tetris/Gravity.h"
#include "Futile/Logging.h"
#include "Futile/Threading.h"
#include "Futile/AutoPtrSupport.h"
#include <boost/bind.hpp>
#include <set>
#include <stdexcept>


namespace Tetris {


using Futile::CreatePoly;
using Futile::LogWarning;
using Futile::ScopedAccessor;
using Futile::ThreadSafe;


struct Game::Impl : public GameImpl::EventHandler
{
    static Futile::ThreadSafe<GameImpl> CreateGame(PlayerType inPlayerType, std::size_t inRowCount, std::size_t inColumnCount)
    {
        if (inPlayerType == Human)
        {
            return HumanGame::Create(inRowCount, inColumnCount);
        }
        else if (inPlayerType == Computer)
        {
            return ComputerGame::Create(inRowCount, inColumnCount);
        }
        throw std::logic_error("Invalid enum value for PlayerType.");
    }

    Impl(PlayerType inPlayerType,
         std::size_t inRowCount,
         std::size_t inColumnCount) :
        mGameImpl(CreateGame(inPlayerType, inRowCount, inColumnCount)),
        mPlayerType(inPlayerType),
        mGravity(new Gravity(mGameImpl)),
        mCenterColumn(static_cast<std::size_t>(0.5 + inColumnCount / 2.0)),
        mBackPtr(0)
    {
    }

    ~Impl()
    {
        GameImpl::UnregisterEventHandler(mGameImpl, this);
    }

    void init(Game * inGame)
    {
        mBackPtr = inGame;
        GameImpl::RegisterEventHandler(mGameImpl, this);
    }

    virtual void onGameStateChanged(GameImpl * )
    {
        EventHandlers::iterator it = mEventHandlers.begin(), end = mEventHandlers.end();
        for (; it != end; ++it)
        {
            Game::EventHandler * eventHandler(*it);
            eventHandler->onGameStateChanged(mBackPtr);
        }
    }

    virtual void onLinesCleared(GameImpl * , int inLineCount)
    {
        EventHandlers::iterator it = mEventHandlers.begin(), end = mEventHandlers.end();
        for (; it != end; ++it)
        {
            Game::EventHandler * eventHandler(*it);
            eventHandler->onLinesCleared(mBackPtr, inLineCount);
        }
    }

    std::string mName;
    ThreadSafe<GameImpl> mGameImpl;
    PlayerType mPlayerType;
    boost::scoped_ptr<Gravity> mGravity;
    std::size_t mCenterColumn;
    Game * mBackPtr;
    typedef std::set<Game::EventHandler*> EventHandlers;
    EventHandlers mEventHandlers;
};


Game::Instances Game::sInstances;


Game::Game(PlayerType inPlayerType, std::size_t inRowCount, std::size_t inColumnCount) :
    mImpl(new Impl(inPlayerType, inRowCount, inColumnCount))
{
    mImpl->init(this);
    sInstances.insert(this);
}


Game::~Game()
{
    sInstances.erase(this);
    mImpl.reset();
}


PlayerType Game::playerType() const
{
    return mImpl->mPlayerType;
}


void Game::RegisterEventHandler(Game * inGame, EventHandler * inEventHandler)
{
    if (sInstances.find(inGame) == sInstances.end())
    {
        LogWarning("Game::RegisterEventHandler: The Game object does not exist!");
        return;
    }

    inGame->mImpl->mEventHandlers.insert(inEventHandler);
}


void Game::UnregisterEventHandler(Game * inGame, EventHandler * inEventHandler)
{
    if (sInstances.find(inGame) == sInstances.end())
    {
        LogWarning("Game::UnregisterEventHandler: The Game no longer exists!");
        return;
    }

    inGame->mImpl->mEventHandlers.erase(inEventHandler);
}


bool Game::Exists(Game * inGame)
{
    return sInstances.find(inGame) != sInstances.end();
}

ThreadSafe<GameImpl> Game::gameImpl() const
{
    return mImpl->mGameImpl;
}


void Game::setPaused(bool inPaused)
{
    ScopedAccessor<GameImpl> rwgame(mImpl->mGameImpl);
    return rwgame->setPaused(inPaused);
}


bool Game::isPaused() const
{
    ScopedAccessor<GameImpl> rgame(mImpl->mGameImpl);
    return rgame->isPaused();
}


GameStateStats Game::stats() const
{
    Futile::ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
    const GameImpl & gameImpl = *game.get();
    const GameState & gameState = gameImpl.gameState();
    return GameStateStats(gameState.numLines(),
                          gameState.numSingles(),
                          gameState.numDoubles(),
                          gameState.numTriples(),
                          gameState.numTetrises(),
                          gameState.currentHeight());
}


void Game::applyLinePenalty(int inNumberOfLinesMadeByOpponent)
{
    ScopedAccessor<GameImpl> rwgame(mImpl->mGameImpl);
    rwgame->applyLinePenalty(inNumberOfLinesMadeByOpponent);
}


//void Game::setActiveBlock(const Block & inBlock)
//{
//    ScopedAccessor<Game> rwgame(mImpl->mGame);
//    Game & game(*rwgame.get());
//    game.setActiveBlock(inBlock);
//}


//void Game::setGameGrid(const Grid & inGrid)
//{
//    ScopedAccessor<Game> rwgame(mImpl->mGame);
//    Game & game(*rwgame.get());
//    game.setGrid(inGrid);
//}


bool Game::isGameOver() const
{
    ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
    return game->isGameOver();
}


int Game::rowCount() const
{
    ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
    return game->rowCount();
}


int Game::columnCount() const
{
    ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
    return game->columnCount();
}


void Game::move(MoveDirection inDirection)
{
    ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
    game->move(inDirection);
}


void Game::rotate()
{
    ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
    game->rotate();
}


void Game::drop()
{
    ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
    game->dropAndCommit();
}


void Game::setStartingLevel(int inLevel)
{
    ScopedAccessor<GameImpl> rwgame(mImpl->mGameImpl);
    rwgame->setStartingLevel(inLevel);
}


int Game::level() const
{
    return ScopedAccessor<GameImpl>(mImpl->mGameImpl)->level();
}


Block Game::activeBlock() const
{
    return ScopedAccessor<GameImpl>(mImpl->mGameImpl)->activeBlock();
}


Grid Game::gameGrid() const
{
    return ScopedAccessor<GameImpl>(mImpl->mGameImpl)->gameGrid();
}


Block Game::getNextBlock()
{
    std::vector<BlockType> blockTypes;
    {
        ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
        game->getFutureBlocks(2, blockTypes);
    }

    if (blockTypes.size() != 2)
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    return Block(blockTypes.back(), Rotation(0), Row(0), Column((columnCount() - mImpl->mCenterColumn)/2));
}


std::vector<Block> Game::getNextBlocks()
{
    std::vector<BlockType> blockTypes;
    int numFutureBlocks = futureBlocksCount();
    {
        ScopedAccessor<GameImpl> game(mImpl->mGameImpl);
        game->getFutureBlocks(1 + numFutureBlocks, blockTypes);
    }

    if (blockTypes.size() != (1 + numFutureBlocks))
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


int Game::futureBlocksCount() const
{
    return ScopedAccessor<GameImpl>(mImpl->mGameImpl)->futureBlocksCount();
}


} // namespace Tetris
