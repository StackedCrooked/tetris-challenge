#include "Poco/Foundation.h"
#include "Controller.h"
#include "Console.h"
#include "TetrisComponent.h"
#include "TetrisElement.h"
#include "Futile/Logger.h"
#include "Futile/Singleton.h"
#include "Futile/MainThread.h"
#include "Futile/MainThreadImpl.h"
#include "Futile/Threading.h"
#include "XULWin/ElementFactory.h"
#include "XULWin/Initializer.h"
#include "XULWin/Windows.h"
#include "XULWin/WinUtils.h"
#include "XULWin/Unicode.h"
#include <stdexcept>
#include <windows.h>


namespace Futile {


class Win32MainThreadImpl : public MainThreadImpl
{
public:
    typedef XULWin::WinAPI::Timer Timer;
    typedef std::list<Action> Actions;

    Win32MainThreadImpl()
    {
        mTimer.start(boost::bind(&Win32MainThreadImpl::processActions, this), 10);
    }


    virtual ~Win32MainThreadImpl()
    {
        mTimer.stop();
    }

    virtual void postAction(Action inAction)
    {
        ScopedWriter<Actions> wactions(mActions);
        Actions & actions = *wactions.get();
        actions.push_back(inAction);
    }

private:
    void processActions()
    {
        ScopedWriter<Actions> wactions(mActions);
        Actions & actions = *wactions.get();
        while (!actions.empty())
        {
            Action & action = *actions.begin();
            action();
            actions.pop_front();
        }
    }

    Timer mTimer;
    ThreadSafe<Actions> mActions;
};


std::auto_ptr<MainThreadImpl> CreateMainThreadImpl()
{
    std::auto_ptr<MainThreadImpl> result;
    result.reset(new Win32MainThreadImpl);
    return result;
}


} // namespace Futile


// Defined in TetrisTestSuite's Driver.cpp
int RunTetrisTestSuite();


int StartProgram(HINSTANCE hInstance)
{
#ifndef NDEBUG // only required when launching from Visual Studio
    // Change the current directory to the XUL Directory
    XULWin::WinAPI::CurrentDirectoryChanger cd("Tetris.xul");

    AttachToConsole();
    int res = RunTetrisTestSuite();
    if (res != 0)
    {
        throw std::runtime_error("TestSuite failed");
    }
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


using namespace Futile;


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        Singleton<Logger>::Initializer theLoggerInitializer;
        MainThread::Initializer theMainThreadInitializer;
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
