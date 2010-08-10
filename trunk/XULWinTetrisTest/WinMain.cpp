#include "TetrisElement.h"
#include "TetrisComponent.h"
#include "Game.h"
#include "Unicode.h"
#include "XULWin/XULRunner.h"
#include "XULWin/Decorator.h"
#include "XULWin/ElementFactory.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/EventListener.h"
#include "XULWin/Element.h"
#include "XULWin/Component.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/EventListener.h"
#include "XULWin/Initializer.h"
#include "XULWin/Window.h"
#include "XULWin/WinUtils.h"
#include "XULWin/Unicode.h"
#include <sstream>
#include <stdexcept>
#include <string>
#include <windows.h>
#include <shellapi.h>



LRESULT ForwardKeyDownMessage(HWND inHWND, WPARAM wParam, LPARAM lParam)
{
    return SendMessage(inHWND, WM_KEYDOWN, wParam, lParam);
}


void run(HMODULE inModuleHandle)
{
    XULWin::ErrorCatcher errorCatcher;
    errorCatcher.disableLogging(true);
    XULWin::XULRunner runner(inModuleHandle);
    XULWin::ElementPtr rootElement = runner.loadXULFromFile("XULWinTetrisTest.xul");
    if (!rootElement)
    {
        XULWin::ReportError("Failed to load the root element");
        return;
    }

    XULWin::Window * wnd = rootElement->component()->downcast<XULWin::Window>();
    if (!wnd)
    {
        XULWin::ReportError("Root element is not of type winodw.");
        return;
    }

    wnd->showModal(XULWin::WindowPos_CenterInScreen);
}


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    XULWin::Initializer initializer(hInstance);
    XULWin::WinAPI::CommonControlsInitializer ccInit;
    XULWin::ElementFactory::Instance().registerElement<Tetris::TetrisElement>();
    run(hInstance);
    return 0;
}
