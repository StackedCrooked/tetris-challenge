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


int SimpleTest()
{
    std::cout << "Running a simple test..." << std::endl;

    int result = 0;    
    ThreadSafeGame threadSafeGame(std::auto_ptr<Game>(new Game));
    while (!ReadOnlyGame(threadSafeGame)->isGameOver())
    {
        std::auto_ptr<GameStateNode> currentNode;
        BlockTypes futureBlocks;
        // Critical section: fetch a clone of the current node and the list of future blocks
        {
            ReadOnlyGame rgame(threadSafeGame);
            const Game & game = *(rgame.get());
            game.getFutureBlocks(2, futureBlocks);
            currentNode = game.currentNode()->clone();
        }
        
        // Prepare just a check
        int currentDepth = currentNode->depth();

        // Setup the player object
        Player p(currentNode, futureBlocks, 1000, 2);
        ChildNodePtr resultNode = p.start(); // Waiting...

        // Just a check
        assert(currentDepth + 1 == resultNode->depth());

        // Critical section: apply results
        {
            WritableGame wgame(threadSafeGame);
            Game & game = *(wgame.get());
            game.currentNode()->addChild(resultNode);
            
            // Move down until done at the end.
            while(game.navigateNodeDown());
            GameState::Stats stats = game.currentNode()->state().stats();
            std::cout << "Lines: " << stats.numLines() << "\r";
        }
    }
    std::cout << "Game over!" << std::endl;
    return ReadOnlyGame(threadSafeGame)->currentNode()->state().stats().numLines();
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
        SimpleTest();
    }
    catch (const std::exception & exc)
    {
        std::cout << "Unhandled exception: " << exc.what() << std::endl;
    }
    std::cout << std::endl << "Press ENTER to quit.";
    std::cin.get();
    return 0;
}
