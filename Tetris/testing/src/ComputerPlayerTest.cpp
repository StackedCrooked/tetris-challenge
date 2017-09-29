#include "TetrisTest.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Game.h"
#include "Futile/Threading.h"
#include <iostream>
#include <string>


namespace testing {


using namespace Tetris;


struct ComputerPlayerTest : public TetrisTest { };


TEST_F(ComputerPlayerTest, All)
{
    enum {
        cMoveSpeed = 100,
        cSearchDepth = 4,
        cSearchWidth = 5,
        cWorkerCount = 6,
        cDuration = 5000
    };

    Game game(20, 10);
    Computer computer(game);

    computer.setMoveSpeed(cMoveSpeed);
    ASSERT_EQ(computer.moveSpeed(), cMoveSpeed);

    computer.setSearchWidth(cSearchWidth);
    ASSERT_EQ(computer.searchWidth(), cSearchWidth);

    computer.setSearchDepth(cSearchDepth);
    ASSERT_EQ(computer.searchDepth(), cSearchDepth);

    computer.setWorkerCount(cWorkerCount);
    ASSERT_EQ(computer.workerCount(), cWorkerCount);

    std::cout << "Let computer work for " << cDuration << "ms" << std::endl;
    Futile::Sleep(cDuration);
}


} // namespace testing
