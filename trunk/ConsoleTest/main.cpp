#include "Game.h"
#include "ErrorHandling.h"
#include "GameState.h"
#include "GameStateNode.h"
#include "Player.h"
#include "Poco/Stopwatch.h"
#include <stdexcept>
#include <iostream>


using namespace Tetris;


std::vector<int> GetParameters(int inDepth, int inWidth)
{
    std::vector<int> result;
    for (int i = 0; i < inDepth; ++i)
    {
        result.push_back(inWidth);
    }
    return result;
}


int Test(const std::vector<int> & inDepths, bool inMultiThreaded)
{
    Game game(20, 10); // 10 rows to have game-over quicker :)
    Player player(&game);
    player.playUntilGameOver(inDepths, inMultiThreaded);
    std::cout << "Blocks: " << game.currentNode().depth() <<  "\tLines: " << game.currentNode().state().stats().mNumLines << "\r";
    std::cout << std::endl;
    return game.currentNode().state().stats().mNumLines;
}


void Test(const std::vector<int> inDepths, size_t inCount, bool inMultiThreaded)
{

    Poco::Stopwatch stopWatch;
    stopWatch.start();
    std::cout << "TEST: " << (inMultiThreaded ? "MULTITHREADED" : "SINGLE THREADED") << std::endl;
    std::cout << "Testing with depths: ";
    for (size_t i = 0; i < inDepths.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << ", ";
        }
        std::cout << inDepths[i];
    }

    std::cout << std::endl;
    int sumLines = 0;
    for (size_t idx = 0; idx != inCount; ++idx)
    {
        sumLines += Test(inDepths, inMultiThreaded);
    }
    std::cout << "AVG line count: " << (int)(0.5 + (float)sumLines/(float)inCount) << std::endl;
    std::cout << "Duration: " << (stopWatch.elapsed() / 1000) << "ms" << std::endl << std::endl;
}


int main()
{
    try
    {
        int repeat = 20;
        //Test(GetDepths(1), repeat, false);
        //Test(GetDepths(1), repeat, true);
        Test(GetParameters(2, 4), repeat, false);
        Test(GetParameters(2, 4), repeat, true);
        Test(GetParameters(3, 3), repeat, false);
        Test(GetParameters(3, 3), repeat, true);
        //Test(GetDepths(4), repeat, false);
        //Test(GetDepths(4), repeat, true);
    }
    catch (const std::exception & exc)
    {
        std::cout << "Unhandled exception: " << exc.what() << std::endl;
    }
    std::cout << std::endl << "Press ENTER to quit.";
    std::cin.get();
    return 0;
}