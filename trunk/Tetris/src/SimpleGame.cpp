#include "Tetris/Config.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/AutoPtrSupport.h"
#include <boost/bind.hpp>
#include <stdexcept>


namespace Tetris {


SimpleGame::SimpleGame(EventHandler * inEventHandler, size_t inRowCount, size_t inColumnCount) :
    mGame(Create<Game>(inRowCount, inColumnCount)),
    mGravity(new Gravity(mGame)),
    mEventHandler(inEventHandler),
    mCenterColumn(static_cast<size_t>(0.5 + inColumnCount / 2.0))
{
    mGravity->setCallback(boost::bind(&EventHandler::onSimpleGameChanged, mEventHandler));
}


SimpleGame::~SimpleGame()
{
}


bool SimpleGame::isGameOver() const
{
    return ScopedReader<Game>(mGame)->isGameOver();
}


int SimpleGame::rowCount() const
{
    return ScopedReader<Game>(mGame)->rowCount();
}


int SimpleGame::columnCount() const
{
    return ScopedReader<Game>(mGame)->columnCount();
}


void SimpleGame::move(Direction inDirection)
{
    ScopedReaderAndWriter<Game> game(mGame);
    game->move(inDirection);
}


void SimpleGame::rotate()
{
    ScopedReaderAndWriter<Game> game(mGame);
    game->rotate();
}


void SimpleGame::drop()
{
    ScopedReaderAndWriter<Game> game(mGame);
    game->drop();
}


int SimpleGame::level() const
{
    return ScopedReader<Game>(mGame)->level();
}


Block SimpleGame::activeBlock() const
{
    return ScopedReader<Game>(mGame)->activeBlock();
}


Grid SimpleGame::gameGrid() const
{
    return ScopedReader<Game>(mGame)->gameGrid();
}


Block SimpleGame::getNextBlock() const
{
    std::vector<BlockType> blockTypes;
    {
        ScopedReader<Game> game(mGame);
        game->getFutureBlocksWithOffset(1, 1, blockTypes);
    }

    if (blockTypes.empty())
    {
        throw std::logic_error("Failed to get the next block from the factory.");
    }

    return Block(blockTypes[0], Rotation(0), Row(0), Column((columnCount() - mCenterColumn)/2));
}


} // namespace Tetris
