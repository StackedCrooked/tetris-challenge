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
    GameController gameController(20, 10);
    Visualizer visualizer(&gameController);
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
