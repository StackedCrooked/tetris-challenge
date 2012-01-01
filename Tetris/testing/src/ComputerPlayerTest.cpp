#include "TetrisTest.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Game.h"
#include "Futile/Threading.h"
#include <iostream>
#include <string>


namespace testing {


using namespace Tetris;


class ComputerPlayerTest : public TetrisTest
{
};


TEST_F(ComputerPlayerTest, All)
{
	static const unsigned cWaitMs = 1000;
    boost::scoped_ptr<Game> game(new Game(20, 10));
    boost::scoped_ptr<Computer> computer(new Computer(*game));
    std::cout << "Let computer work for " << cWaitMs << "ms" << std::endl;
    Futile::Sleep(cWaitMs);
    computer.reset();
    game.reset();
}


} // namespace testing
