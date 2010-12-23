#include "Tetris/SimpleGame.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Game.h"
#include "Tetris/AutoPtrSupport.h"


namespace Tetris {


SimpleGame::SimpleGame(size_t inRowCount, size_t inColumnCount) :
    mGame(Create<Game>(inRowCount, inColumnCount)),
    mGravity(),
    mComputerPlayer(),
    mComputerMoveSpeed(0)
{

}


SimpleGame::~SimpleGame()
{
}


void SimpleGame::enableGravity(bool inEnabled)
{
    if (inEnabled)
    {
        if (!mGravity)
        {
            mGravity.reset(new Gravity(mGame));
        }
    }
    else
    {
        mGravity.reset();
    }
}


void SimpleGame::enableComputerPlayer(bool inEnabled)
{
    if (inEnabled)
    {
        if (!mComputerPlayer)
        {
            mComputerPlayer.reset(new ComputerPlayer(mGame, CreatePoly<Evaluator, Balanced>(), 6, 6, 1));

            if (mComputerMoveSpeed)
            {
                mComputerPlayer->setMoveSpeed(mComputerMoveSpeed);
            }
        }
    }
    else
    {
        mComputerPlayer.reset();
    }
}


void SimpleGame::setComputerMoveSpeed(int inNumberOfMovesPerSecond)
{
    mComputerMoveSpeed = inNumberOfMovesPerSecond;
    if (mComputerPlayer)
    {
        mComputerPlayer->setMoveSpeed(mComputerMoveSpeed);
    }
}


bool SimpleGame::isGameOver() const
{
    return ScopedReader<Game>(mGame)->isGameOver();
}


void SimpleGame::getSize(int & outColoumCount, int & outRowCount)
{
    ScopedReader<Game> game(mGame);
    outColoumCount = game->columnCount();
    outRowCount = game->rowCount();
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


void SimpleGame::setLevel(int inLevel)
{
    return ScopedReaderAndWriter<Game>(mGame)->setLevel(inLevel);
}


Block SimpleGame::activeBlock() const
{
    return ScopedReader<Game>(mGame)->activeBlock();
}


Grid SimpleGame::gameGrid() const
{
    return ScopedReader<Game>(mGame)->gameGrid();
}


std::vector<Block> SimpleGame::getNextBlocks(size_t inCount) const
{
    std::vector<BlockType> blockTypes;
    {
        ScopedReader<Game> game(mGame);
        game->getFutureBlocksWithOffset(1, inCount, blockTypes);
    }

    std::vector<Block> result;
    for (unsigned i = 0; i < blockTypes.size(); ++i)
    {
        result.push_back(Block(blockTypes[i], Rotation(0), Row(0), Column(0)));
    }
    return result;
}


} // namespace Tetris
