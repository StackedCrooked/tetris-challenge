#include "TetrisElement.h"
#include "GameTimer.h"
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
#include <boost/scoped_ptr.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <windows.h>
#include <shellapi.h>


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    XULWin::Initializer initializer(hInstance);
    XULWin::WinAPI::CommonControlsInitializer ccInit;
    XULWin::ElementFactory::Instance().registerElement<Tetris::TetrisElement>();

    XULWin::ErrorCatcher errorCatcher;
    errorCatcher.disableLogging(true);

    XULWin::XULRunner runner(hInstance);
    XULWin::ElementPtr rootElement = runner.loadXULFromFile("XULWinTetrisTest.xul");
    if (!rootElement)
    {
        XULWin::ReportError("Failed to load the root element");
        return 1;
    }

    XULWin::Window * wnd = rootElement->component()->downcast<XULWin::Window>();
    if (!wnd)
    {
        XULWin::ReportError("Root element is not of type winodw.");
        return 1;
    }

    boost::scoped_ptr<Tetris::GameTimer> timedGame;

    std::vector<Tetris::TetrisElement *> tetrisElements;
    rootElement->getElementsByType<Tetris::TetrisElement>(tetrisElements);
    for (size_t idx = 0; idx != tetrisElements.size(); ++idx)
    {
        Tetris::TetrisComponent * tetris = tetrisElements[idx]->component()->downcast<Tetris::TetrisComponent>();
        timedGame.reset(new Tetris::GameTimer(&tetris->getGame()));
        break;
    }

	// TODO: Make multi-threaded!!
    timedGame->start();

    wnd->showModal(XULWin::WindowPos_CenterInScreen);
    return 0;
}
