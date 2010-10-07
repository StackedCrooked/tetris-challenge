#include "Controller.h"
#include "TetrisComponent.h"
#include "TetrisElement.h"
#include "XULWin/ElementFactory.h"
#include "XULWin/Initializer.h"
#include "XULWin/Windows.h"
#include "XULWin/Unicode.h"
#include <stdexcept>
#include <windows.h>


int StartProgram(HINSTANCE hInstance)
{
#ifndef NDEBUG // only required when launching from Visual Studio
    // Change the current directory to the XUL Directory
    XULWin::WinAPI::CurrentDirectoryChanger cd("Tetris.xul");
#endif

    XULWin::Initializer initializer(hInstance);

    // Register the XML tag "tetris" in the factory lookup table.
    XULWin::ElementFactory::Instance().registerElement<Tetris::TetrisElement>();

    // Create the Controller object. Starts the game.
    Tetris::Controller controller(hInstance);

    // Run the game
    controller.run();
    return 0;
}


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        return StartProgram(hInstance);
    }
    catch (const std::exception & inError)
    {
        ::MessageBox(0, XULWin::ToUTF16(inError.what()).c_str(), TEXT("Tetris"), MB_OK);
    }
    catch (...)
    {
        ::MessageBox(0, TEXT("Program is terminated due to unhandled and unknown exception."), L"Tetris", MB_OK);
    }
    return 0;
}
