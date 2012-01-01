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
    Game game(20, 10);
    Computer computer(game);
    std::cout << "Let computer work for 10s" << std::endl;
    Futile::Sleep(10000);
}


} // namespace testing
