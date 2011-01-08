#include "Tetris/Config.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/Gravity.h"
#include "Tetris/Threading.h"
#include "Tetris/AutoPtrSupport.h"
#include <stdexcept>


namespace Tetris {


struct SimpleGame::SimpleGameImpl : public Game::EventHandler
{
    SimpleGameImpl(SimpleGame::EventHandler * inSimpleGameEventHandler,
                   size_t inRowCount,
                   size_t inColumnCount) :
        mGame(Create<Game>(this, inRowCount, inColumnCount)),
        mGravity(new Gravity(mGame)),
        mSimpleGameEventHandler(inSimpleGameEventHandler),
        mCenterColumn(static_cast<size_t>(0.5 + inColumnCount / 2.0))
    {
    }


    virtual void onGameChanged()
    {
        mSimpleGameEventHandler->onSimpleGameChanged();
    }

    ThreadSafe<Game> mGame;
    boost::scoped_ptr<Gravity> mGravity;
    SimpleGame::EventHandler * mSimpleGameEventHandler;
    std::size_t mCenterColumn;
};


SimpleGame::SimpleGame(SimpleGame::EventHandler * inEventHandler, size_t inRowCount, size_t inColumnCount) :
    mImpl(new SimpleGameImpl(inEventHandler, inRowCount, inColumnCount))
{
}


SimpleGame::~SimpleGame()
{
}


bool SimpleGame::isGameOver() const
{
    return ScopedReader<Game>(mImpl->mGame)->isGameOver();
}


int SimpleGame::rowCount() const
{
    return ScopedReader<Game>(mImpl->mGame)->rowCount();
}


int SimpleGame::columnCount() const
{
    return ScopedReader<Game>(mImpl->mGame)->columnCount();
}


void SimpleGame::move(Direction inDirection)
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
        game->getFutureBlocksWithOffset(1, 1, blockTypes);
    }

    if (blockTypes.empty())
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    return Block(blockTypes[0], Rotation(0), Row(0), Column((columnCount() - mImpl->mCenterColumn)/2));
}


} // namespace Tetris
