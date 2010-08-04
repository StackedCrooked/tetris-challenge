#include "Game.h"
#include "GameState.h"
#include "GameStateNode.h"
#include "Player.h"
#include <stdexcept>
#include <iostream>


using namespace Tetris;


std::vector<int> GetDepths(int inDepth)
{
    std::vector<int> result;
    for (int i = 0; i < inDepth; ++i)
    {
        result.push_back(inDepth);
    }
    return result;
}


int Test(const std::vector<int> inDepths)
{
    Game game(20, 10);
    Player player(&game);

    while (!game.isGameOver())
    {
        player.move(inDepths);
        game.setCurrentNode(game.currentNode().bestChild(inDepths.size()));
    }
    std::cout << "Blocks: " << game.currentNode().depth() <<  "\tLines: " << game.currentNode().state().stats().mNumLines << "\r";
    std::cout << std::endl;
    return game.currentNode().state().stats().mNumLines;
}


void Test(const std::vector<int> inDepths, size_t inCount)
{
    std::cout << "Testing with depths: ";
    for (int i = 0; i < inDepths.size(); ++i)
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
        sumLines += Test(inDepths);
    }
    std::cout << "AVG line count: " << (int)(0.5 + (float)sumLines/(float)inCount) << std::endl << std::endl;
}


int main()
{
    try
    {
        Test(GetDepths(1), 5);
        Test(GetDepths(2), 5);
        Test(GetDepths(3), 5);
        Test(GetDepths(4), 5);
        Test(GetDepths(5), 5);
    }
    catch (const std::exception & exc)
    {
        std::cout << "Unhandled exception: " << exc.what() << std::endl;
    }
    std::cout << std::endl << "Press ENTER to quit.";
    std::cin.get();
    return 0;
}