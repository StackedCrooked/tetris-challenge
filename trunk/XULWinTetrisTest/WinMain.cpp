#include "Controller.h"
#include "TetrisComponent.h"
#include "TetrisElement.h"
#include "XULWin/ElementFactory.h"
#include "XULWin/Initializer.h"
#include "XULWin/Windows.h"
#include "XULWin/Unicode.h"
#include <stdexcept>
#include <windows.h>
#include <boost/scoped_ptr.hpp>


int StartProgram(HINSTANCE hInstance)
{
    boost::scoped_ptr<XULWin::WinAPI::CurrentDirectoryChanger> cd;
    if (XULWin::WinAPI::getCurrentDirectory().find("Tetris.xul") == std::string::npos)
    {
	    // Change the current directory to the XUL Directory
        cd.reset(new XULWin::WinAPI::CurrentDirectoryChanger("Tetris.xul"));
    }

    XULWin::Initializer initializer(hInstance);

    // Register the XML tag "tetris" in the factory lookup table.
    XULWin::ElementFactory::Instance().registerElement<Tetris::TetrisElement>();

    // Create the Controller object. Starts the game.
    Tetris::Controller controller(hInstance);

    // Run the game
    controller.run();
    
    // Closing
    controller.setQuitFlag();
    controller.joinAllThreads();
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
        ::MessageBox(0, XULWin::ToUTF16(inError.what()).c_str(), TEXT("XULWin Tetris Component"), MB_OK);
    }
    catch (...)
    {
        ::MessageBox(0, TEXT("Program is terminated due to unhandled and unknown exception."), L"XULWin Tetris Component", MB_OK);
    }
    return 0;
}
