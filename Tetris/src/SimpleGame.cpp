#include "Tetris/Config.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/Gravity.h"
#include "Tetris/Threading.h"
#include "Tetris/AutoPtrSupport.h"
#include <boost/bind.hpp>
#include <stdexcept>


namespace Tetris {


struct SimpleGame::Impl
{
    Impl(size_t inRowCount,
                   size_t inColumnCount) :
        mGame(new HumanGame(inRowCount, inColumnCount)),
        mGravity(new Gravity(mGame)),
        mCenterColumn(static_cast<size_t>(0.5 + inColumnCount / 2.0)),
        mSimpleGame(0)
    {
    }

    void init(SimpleGame * inSimpleGame)
    {
        mSimpleGame = inSimpleGame;
        ScopedReaderAndWriter<Game> rwgame(mGame);
        Game & game(*rwgame.get());
        game.OnChanged.connect(boost::bind(&SimpleGame::Impl::onChanged, this, _1));
    }

    void onChanged(Game & )
    {
        mSimpleGame->OnChanged(*mSimpleGame);
    }

    ThreadSafe<Game> mGame;
    boost::scoped_ptr<Gravity> mGravity;
    std::size_t mCenterColumn;
    SimpleGame * mSimpleGame;
};


SimpleGame::SimpleGame(size_t inRowCount, size_t inColumnCount) :
    mImpl(new Impl(inRowCount, inColumnCount))
{
    mImpl->init(this);
}


SimpleGame::~SimpleGame()
{
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
