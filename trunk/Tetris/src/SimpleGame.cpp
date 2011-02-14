#include "Tetris/Config.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/Gravity.h"
#include "Tetris/Logging.h"
#include "Tetris/Threading.h"
#include "Tetris/AutoPtrSupport.h"
#include <boost/bind.hpp>
#include <set>
#include <stdexcept>


namespace Tetris {


struct SimpleGame::Impl : public Game::EventHandler,
                          public ComputerPlayer::Tweaker
{
    typedef SimpleGame::BackReference BackReference;

    static std::auto_ptr<Game> CreateGame(size_t inRowCount, size_t inColumnCount, PlayerType inPlayerType)
    {
        if (inPlayerType == PlayerType_Human)
        {
            return CreatePoly<Game, HumanGame>(inRowCount, inColumnCount);
        }
        else if (inPlayerType == PlayerType_Computer)
        {
            return CreatePoly<Game, ComputerGame>(inRowCount, inColumnCount);
        }
        throw std::logic_error("Invalid enum value for PlayerType.");
    }

    Impl(size_t inRowCount,
         size_t inColumnCount,
         PlayerType inPlayerType) :
        mGame(CreateGame(inRowCount, inColumnCount, inPlayerType)),
        mPlayerType(inPlayerType),
        mComputerPlayer(),
        mGravity(new Gravity(mGame)),
        mCenterColumn(static_cast<size_t>(0.5 + inColumnCount / 2.0)),
        mSimpleGame(0),
        mBackReference(0)
    {
        if (inPlayerType == PlayerType_Computer)
        {
            std::auto_ptr<Evaluator> evaluator(CreatePoly<Evaluator, MakeTetrises>());
            mComputerPlayer.reset(new ComputerPlayer(mGame, evaluator, 6, 6, 8));
            mComputerPlayer->setTweaker(this);
            mComputerPlayer->setMoveSpeed(100);
        }
    }

    ~Impl()
    {
        Game::UnregisterEventHandler(mGame, this);
    }

    virtual std::auto_ptr<Evaluator> updateInfo(const GameState & inGameState,
                                                int & outSearchDepth,
                                                int & outSearchWidth)
    {
        int firstRow = inGameState.firstOccupiedRow();
        int rowCount = inGameState.grid().rowCount();
//        if (0.5 * rowCount < float(firstRow))
//        {
            outSearchDepth = 8;
            outSearchWidth = 5;
            return CreatePoly<Evaluator, MakeTetrises>();
//        }
//        else if (0.6 * rowCount < float(firstRow))
//        {
//            outSearchDepth = 5;
//            outSearchWidth = 5;
//            return CreatePoly<Evaluator, Balanced>();
//        }
//        else
//        {
//            outSearchDepth = 4;
//            outSearchWidth = 4;
//            return CreatePoly<Evaluator, Survival>();
//        }
    }

    void init(SimpleGame * inSimpleGame)
    {
        mSimpleGame = inSimpleGame;
        Game::RegisterEventHandler(mGame, this);
    }

    virtual void onGameStateChanged(Game * )
    {
        EventHandlers::iterator it = mEventHandlers.begin(), end = mEventHandlers.end();
        for (; it != end; ++it)
        {
            SimpleGame::EventHandler * eventHandler(*it);
            eventHandler->onGameStateChanged(mSimpleGame);
        }
    }

    virtual void onLinesCleared(Game * , int inLineCount)
    {
        EventHandlers::iterator it = mEventHandlers.begin(), end = mEventHandlers.end();
        for (; it != end; ++it)
        {
            SimpleGame::EventHandler * eventHandler(*it);
            eventHandler->onLinesCleared(mSimpleGame, inLineCount);
        }
    }

    ThreadSafe<Game> mGame;
    PlayerType mPlayerType;
    boost::scoped_ptr<ComputerPlayer> mComputerPlayer;
    boost::scoped_ptr<Gravity> mGravity;
    std::size_t mCenterColumn;
    SimpleGame * mSimpleGame;
    BackReference * mBackReference;
    typedef std::set<SimpleGame::EventHandler*> EventHandlers;
    EventHandlers mEventHandlers;
};


SimpleGame::Instances SimpleGame::sInstances;


SimpleGame::SimpleGame(size_t inRowCount, size_t inColumnCount, PlayerType inPlayerType) :
    mImpl(new Impl(inRowCount, inColumnCount, inPlayerType))
{
    mImpl->init(this);
    sInstances.insert(this);
}


SimpleGame::~SimpleGame()
{
    sInstances.erase(this);
    delete mImpl;
}


void SimpleGame::setBackReference(BackReference * inBackReference)
{
    mImpl->mBackReference = inBackReference;
}


SimpleGame::BackReference * SimpleGame::getBackReference()
{
    return mImpl->mBackReference;
}


const SimpleGame::BackReference * SimpleGame::getBackReference() const
{
    return mImpl->mBackReference;
}


PlayerType SimpleGame::playerType() const
{
    return mImpl->mPlayerType;
}


void SimpleGame::RegisterEventHandler(SimpleGame * inSimpleGame, EventHandler * inEventHandler)
{
    if (sInstances.find(inSimpleGame) == sInstances.end())
    {
        LogWarning("SimpleGame::RegisterEventHandler: The SimpleGame object does not exist!");
        return;
    }

    inSimpleGame->mImpl->mEventHandlers.insert(inEventHandler);
}


void SimpleGame::UnregisterEventHandler(SimpleGame * inSimpleGame, EventHandler * inEventHandler)
{
    if (sInstances.find(inSimpleGame) == sInstances.end())
    {
        LogWarning("SimpleGame::UnregisterEventHandler: The SimpleGame no longer exists!");
        return;
    }

    inSimpleGame->mImpl->mEventHandlers.erase(inEventHandler);
}


bool SimpleGame::Exists(SimpleGame * inSimpleGame)
{
    return sInstances.find(inSimpleGame) != sInstances.end();
}


ThreadSafe<Game> SimpleGame::game() const
{
    return mImpl->mGame;
}


void SimpleGame::setPaused(bool inPaused)
{
    LogInfo(MakeString() << "SimpleGame::setPaused: " << inPaused);
    ScopedReaderAndWriter<Game> rwgame(mImpl->mGame);
    return rwgame->setPaused(inPaused);
}


bool SimpleGame::isPaused() const
{
    ScopedReader<Game> rgame(mImpl->mGame);
    return rgame->isPaused();
}


GameStateStats SimpleGame::stats() const
{
    ScopedReader<Game> rgame(mImpl->mGame);
    const GameState & gameState(rgame->gameState());
    return GameStateStats(gameState.numLines(),
                          gameState.numSingles(),
                          gameState.numDoubles(),
                          gameState.numTriples(),
                          gameState.numTetrises());
}


void SimpleGame::applyLinePenalty(int inNumberOfLinesMadeByOpponent)
{
    ScopedReaderAndWriter<Game> rwgame(mImpl->mGame);
    rwgame->applyLinePenalty(inNumberOfLinesMadeByOpponent);
}


void SimpleGame::setActiveBlock(const Block & inBlock)
{
    ScopedReaderAndWriter<Game> rwgame(mImpl->mGame);
    Game & game(*rwgame.get());
    game.setActiveBlock(inBlock);
}

void SimpleGame::setGameGrid(const Grid & inGrid)
{
    ScopedReaderAndWriter<Game> rwgame(mImpl->mGame);
    Game & game(*rwgame.get());
    game.setGrid(inGrid);
}


bool SimpleGame::isGameOver() const
{
    ScopedReader<Game> game(mImpl->mGame);
    return game->isGameOver();
}


int SimpleGame::rowCount() const
{
    ScopedReader<Game> game(mImpl->mGame);
    return game->rowCount();
}


int SimpleGame::columnCount() const
{
    ScopedReader<Game> game(mImpl->mGame);
    return game->columnCount();
}


void SimpleGame::move(MoveDirection inDirection)
{
    ScopedReaderAndWriter<Game> game(mImpl->mGame);
    game->move(inDirection);
}


void SimpleGame::rotate()
{
    ScopedReaderAndWriter<Game> game(mImpl->mGame);
    game->rotate();
}


void SimpleGame::drop()
{
    ScopedReaderAndWriter<Game> game(mImpl->mGame);
    game->drop();
}


void SimpleGame::setLevel(int inLevel)
{
    ScopedReaderAndWriter<Game> rwgame(mImpl->mGame);
    rwgame->setLevel(inLevel);
}


int SimpleGame::level() const
{
    return ScopedReader<Game>(mImpl->mGame)->level();
}


Block SimpleGame::activeBlock() const
{
    return ScopedReader<Game>(mImpl->mGame)->activeBlock();
}


Grid SimpleGame::gameGrid() const
{
    return ScopedReader<Game>(mImpl->mGame)->gameGrid();
}


Block SimpleGame::getNextBlock() const
{
    std::vector<BlockType> blockTypes;
    {
        ScopedReader<Game> game(mImpl->mGame);
        game->getFutureBlocks(2, blockTypes);
    }

    if (blockTypes.size() != 2)
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    return Block(blockTypes.back(), Rotation(0), Row(0), Column((columnCount() - mImpl->mCenterColumn)/2));
}


} // namespace Tetris
