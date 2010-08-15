#include "Game.h"
#include "GameState.h"
#include "GameStateNode.h"
#include "Visualizer.h"
#include "Unicode.h"
#include <stdexcept>
#include <string>


using namespace Tetris;


void StartTest()
{   
    BlockTypes blockTypes;
    BlockFactory blockFactory;
    for (size_t idx = 0; idx < 100000; ++idx)
    {
        blockTypes.push_back(blockFactory.getNext());
    }
    ThreadSafeGame threadSafeGame(std::auto_ptr<Game>(new Game(20, 10, blockTypes)));
    Visualizer visualizer(threadSafeGame);
    visualizer.show();
}


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        StartTest();
    }
    catch (const std::exception & inException)
    {
        std::wstring utf16Message = ToUTF16(inException.what());
        MessageBox(0, utf16Message.c_str(), L"Tetris Tester", MB_OK);
        return 1;
    }
    return 0;
}
