#include "Game.h"
#include "ErrorHandling.h"
#include "GameState.h"
#include "GameStateNode.h"
#include "Player.h"
#include "Poco/Stopwatch.h"
#include <stdexcept>
#include <iostream>
#include <windows.h>


using namespace Tetris;


int round(float value)
{
  return (int)(0.5 + value);
}


int median(const std::vector<int> & inValues)
{
    if (inValues.empty())
    {
        throw std::invalid_argument("Can't calculate median of empty set.");
    }

    if (inValues.size() == 1)
    {
        return inValues[0];
    }

    std::vector<int> copyOfValues(inValues);
    std::sort(copyOfValues.begin(), copyOfValues.end());
    int size = copyOfValues.size();
    if (size %2 == 0)
    {
        float v1 = static_cast<float>(copyOfValues[size/2 - 1]);
        float v2 = static_cast<float>(copyOfValues[size/2]);
        return round(v1 + v2);
    }
    return copyOfValues[size/2];
}


std::vector<int> GetParameters(int inDepth, int inWidth)
{
    std::vector<int> result;
    for (int i = 0; i < inDepth; ++i)
    {
        result.push_back(inWidth);
    }
    return result;
}


int Test(const std::vector<int> & inWidths, const Game & inGame)
{
    std::auto_ptr<Game> gameCopy(inGame.clone());
    Player player(gameCopy.get());
    player.setLogger(std::cout);
    player.playUntilGameOver(inWidths);
    std::cout << "Blocks: " << gameCopy->currentNode()->depth() <<  "\tLines: " << gameCopy->currentNode()->state().stats().numLines() << "\r";
    std::cout << std::endl;
    return gameCopy->currentNode()->state().stats().numLines();
}


void Test(const std::vector<int> inWidths, size_t inCount, const Game & game)
{
    CheckArgument(inCount > 0, "Invalid count");
    Poco::Stopwatch stopWatch;
    stopWatch.start();
    std::cout << "Testing with retention counts: ";
    std::vector<int> lineCounts;
    for (size_t i = 0; i < inWidths.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << ", ";
        }
        std::cout << inWidths[i];
    }

    std::cout << std::endl;
    int sumLines = 0;
    for (size_t idx = 0; idx != inCount; ++idx)
    {
        lineCounts.push_back(Test(inWidths, game));
        sumLines += lineCounts.back();
    }

    int avgLines = (int)(0.5 + (float)sumLines/(float)inCount);
    std::cout << "Average line count: " << avgLines << std::endl;

    if (!lineCounts.empty())
    {
        std::sort(lineCounts.begin(), lineCounts.end());
        std::cout << "Median line count: " << lineCounts[lineCounts.size()/2] << std::endl;
    }

    int duration = (int)(stopWatch.elapsed() / 1000);
    std::cout << "Duration: " << duration << "ms" << std::endl;

    if (avgLines != 0)
    {
        std::cout << "Duration/line: " << duration/avgLines << std::endl << std::endl;
    }
}


int GetCPUCount()
{    
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return static_cast<int>(sysinfo.dwNumberOfProcessors);
}


int main()
{
    try
    {        
        Game game(20, 10);
        // Generate 100000 blocks in advance.
        
        int repeat = 1;
        game.reserveBlocks(100 * 1000);
        Test(GetParameters(2, 4), repeat, game);
        Test(GetParameters(3, 4), repeat, game);
        Test(GetParameters(4, 4), repeat, game);
        Test(GetParameters(5, 4), repeat, game);
        Test(GetParameters(6, 4), repeat, game);
    }
    catch (const std::exception & exc)
    {
        std::cout << "Unhandled exception: " << exc.what() << std::endl;
    }
    std::cout << std::endl << "Press ENTER to quit.";
    std::cin.get();
    return 0;
}
