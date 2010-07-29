#include "Visualizer.h"
#include "Unicode.h"
#include <stdexcept>
#include <string>
#include <windows.h>


using namespace Tetris;


void StartTest()
{
    // do stuff...
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
