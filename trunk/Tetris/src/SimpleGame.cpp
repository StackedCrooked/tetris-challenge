#include "Tetris/Config.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/Gravity.h"
#include "Tetris/Threading.h"
#include "Tetris/AutoPtrSupport.h"
#include <stdexcept>


namespace Tetris {


struct SimpleGame::SimpleGameImpl
{
    SimpleGameImpl(size_t inRowCount,
                   size_t inColumnCount) :
        mGame(Create<HumanGame>(inRowCount, inColumnCount)),
        mGravity(new Gravity(mGame)),
        mCenterColumn(static_cast<size_t>(0.5 + inColumnCount / 2.0))
    {
    }

    ThreadSafe<HumanGame> mGame;
    boost::scoped_ptr<Gravity> mGravity;
    std::size_t mCenterColumn;
};


SimpleGame::SimpleGame(size_t inRowCount, size_t inColumnCount) :
    mImpl(new SimpleGameImpl(inRowCount, inColumnCount))
{
}


SimpleGame::~SimpleGame()
{
}


bool SimpleGame::checkDirty()
{
    ScopedReaderAndWriter<HumanGame> game(mImpl->mGame);
    return game->checkDirty();
}


bool SimpleGame::isGameOver() const
{
    ScopedReader<HumanGame> game(mImpl->mGame);
    return game->isGameOver();
}


int SimpleGame::rowCount() const
{
    ScopedReader<HumanGame> game(mImpl->mGame);
    return game->rowCount();
}


int SimpleGame::columnCount() const
{
    ScopedReader<HumanGame> game(mImpl->mGame);
    return game->columnCount();
}


void SimpleGame::move(Direction inDirection)
{
    ScopedReaderAndWriter<HumanGame> game(mImpl->mGame);
    game->move(inDirection);
}


void SimpleGame::rotate()
{
    ScopedReaderAndWriter<HumanGame> game(mImpl->mGame);
    game->rotate();
}


void SimpleGame::drop()
{
    ScopedReaderAndWriter<HumanGame> game(mImpl->mGame);
    game->drop();
}


int SimpleGame::level() const
{
    return ScopedReader<HumanGame>(mImpl->mGame)->level();
}


Block SimpleGame::activeBlock() const
{
    return ScopedReader<HumanGame>(mImpl->mGame)->activeBlock();
}


Grid SimpleGame::gameGrid() const
{
    return ScopedReader<HumanGame>(mImpl->mGame)->gameGrid();
}


Block SimpleGame::getNextBlock() const
{
    std::vector<BlockType> blockTypes;
    {
        ScopedReader<HumanGame> game(mImpl->mGame);
        game->getFutureBlocks(2, blockTypes);
    }

    if (blockTypes.size() != 2)
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    return Block(blockTypes.back(), Rotation(0), Row(0), Column((columnCount() - mImpl->mCenterColumn)/2));
}


} // namespace Tetris
