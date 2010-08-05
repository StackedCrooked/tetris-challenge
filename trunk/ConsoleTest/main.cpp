#include "Game.h"
#include "ErrorHandling.h"
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
        size_t depth = inDepths.size();
        GameStateNode * child = game.currentNode().bestChild(depth);
        while (!child && depth > 0)
        {
            child = game.currentNode().bestChild(depth--);
        }
        CheckCondition(child != 0, "No child!?");
        CheckCondition(inDepths.size() == depth || child->state().isGameOver(), "Should have been Game Over");
        game.setCurrentNode(child);
    }
    std::cout << "Blocks: " << game.currentNode().depth() <<  "\tLines: " << game.currentNode().state().stats().mNumLines << "\r";
    std::cout << std::endl;
    return game.currentNode().state().stats().mNumLines;
}


void Test(const std::vector<int> inDepths, size_t inCount)
{
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
        sumLines += Test(inDepths);
    }
    std::cout << "AVG line count: " << (int)(0.5 + (float)sumLines/(float)inCount) << std::endl << std::endl;
}


int main()
{
    try
    {
        int repeat = 5;
        Test(GetDepths(1), repeat);
        Test(GetDepths(2), repeat);
        Test(GetDepths(3), repeat);
        Test(GetDepths(4), repeat);
        Test(GetDepths(5), repeat);
    }
    catch (const std::exception & exc)
    {
        std::cout << "Unhandled exception: " << exc.what() << std::endl;
    }
    std::cout << std::endl << "Press ENTER to quit.";
    std::cin.get();
    return 0;
}