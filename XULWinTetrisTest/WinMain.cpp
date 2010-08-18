#include "TetrisElement.h"
#include "Game.h"
#include "GameCommandQueue.h"
#include "Player.h"
#include "TimedGame.h"
#include "TetrisComponent.h"
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


boost::scoped_ptr<Tetris::GameCommandQueue> gCommander;
boost::scoped_ptr<boost::thread> gThread;


namespace Tetris
{

    void ProcessKey(int inKey)
    {
        if (inKey == VK_DELETE)
        {
            BlockTypes blockTypes;
            std::auto_ptr<GameStateNode> clonedGameState;

            // Critical section
            {
                WritableGame game(gCommander->threadSafeGame());
                game->getFutureBlocks(19, blockTypes);
                clonedGameState = game->currentNode()->clone();
            }

            int currentGameDepth = clonedGameState->depth();
            TimedNodePopulator populator(clonedGameState, blockTypes, 1000);
            populator.start();                
            ChildNodePtr bestLastChild = populator.getBestChild();
            GameStateNode * bestFirstChild = bestLastChild.get();
            while (bestFirstChild->depth() > currentGameDepth + 1)
            {
                bestFirstChild = bestFirstChild->parent();
            }
            DestroyInferiorChildren(bestFirstChild, bestLastChild.get());
            
            ChildNodePtr firstChild;
            for (ChildNodes::iterator it = populator.node()->children().begin(); it != populator.node()->children().end(); ++it)
            {
                ChildNodePtr child = *it;
                if (child.get() == bestFirstChild)
                {
                    firstChild = child;
                    break;
                }
            }

            assert(firstChild);

            // Critical section
            {
                WritableGame game(gCommander->threadSafeGame());
                assert(game->currentNode()->children().empty()); // FOR NOW!
                game->currentNode()->children().insert(ChildNodePtr(firstChild));
                while(game->navigateNodeDown());
            }

        }
    }

}


LRESULT SaySomething(const std::string & inMessage)
{
    std::wstring utf16Message = Tetris::ToUTF16(inMessage);
    ::MessageBox(0, utf16Message.c_str(), L"Tetris", MB_OK);
    return XULWin::cHandled;
}


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
    XULWin::ScopedEventListener listener;
    listener.connect(rootElement->getElementById("yesButton"), boost::bind(SaySomething, "Yes!!!"));
    listener.connect(rootElement->getElementById("noButton"), boost::bind(SaySomething, "No :("));

    if (XULWin::Element * yesButtonEl = rootElement->getElementById("yesButton"))
    {
        
    }

    Tetris::TetrisComponent * tetrisComponent(0);
    {
        std::vector<Tetris::TetrisElement *> tetrisElements;
        rootElement->getElementsByType<Tetris::TetrisElement>(tetrisElements);
        for (size_t idx = 0; idx != tetrisElements.size(); ++idx)
        {
            tetrisComponent = tetrisElements[idx]->component()->downcast<Tetris::TetrisComponent>();
            break;
        }
    }

    if (!tetrisComponent)
    {
        return 0;
    }

    // Add the timer
    Tetris::TimedGame timedGame(tetrisComponent->getThreadSafeGame());
    timedGame.start();


    // Init the command queue
    gCommander.reset(new Tetris::GameCommandQueue(tetrisComponent->getThreadSafeGame()));

    // Register the keyboard listener
    tetrisComponent->OnKeyboardPressed.connect(boost::bind(Tetris::ProcessKey, _1));

    wnd->showModal(XULWin::WindowPos_CenterInScreen);
    gCommander.reset();
    return 0;
}
