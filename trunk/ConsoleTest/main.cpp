#include "ErrorHandling.h"
#include "Game.h"
#include "GameState.h"
#include "GameStateNode.h"
#include "ThreadSafeGame.h"
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
    int result = 0;
    ThreadSafeGame threadSafeGame(inGame.clone());
    {
        WritableGame game(threadSafeGame);
        game->reserveBlocks(100000);
    }
    {
        Player player(threadSafeGame);
        player.setLogger(std::cout);
        player.playUntilGameOver(inWidths);
    }
    {
        ReadOnlyGame game(threadSafeGame);
        std::cout << "Blocks: " << game->currentNode()->depth() <<  "\tLines: " << game->currentNode()->state().stats().numLines() << "\r";
        std::cout << std::endl;
        result = game->currentNode()->state().stats().numLines();
    }
    return result;
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
        BlockTypes blockTypes;
        BlockFactory blockFactory;
        for (size_t idx = 0; idx < 100000; ++idx)
        {
            blockTypes.push_back(blockFactory.getNext());
        }        
        int repeat = 1;
        Test(GetParameters(2, 2), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(2, 3), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(2, 4), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(3, 2), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(3, 3), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(3, 4), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(4, 2), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(4, 3), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(4, 4), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(5, 2), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(5, 3), repeat, Game(20, 10, blockTypes));
        Test(GetParameters(5, 4), repeat, Game(20, 10, blockTypes));
    }
    catch (const std::exception & exc)
    {
        std::cout << "Unhandled exception: " << exc.what() << std::endl;
    }
    std::cout << std::endl << "Press ENTER to quit.";
    std::cin.get();
    return 0;
}
