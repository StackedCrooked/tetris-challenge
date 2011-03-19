#include "Futile/MainThread.h"
#include "Futile/MainThreadImpl.h"
#include <memory>
#include <stdexcept>


namespace Futile {


void InvokeLater(const Action & inAction)
{
    MainThread::Instance().postAction(inAction);
}


MainThread::Initializer::Initializer()
{
    if (MainThread::sInstance)
    {
        throw std::logic_error("MainThread::sInstance is already set.");
    }
    MainThread::sInstance = new MainThread;
}


MainThread::Initializer::~Initializer()
{
    delete MainThread::sInstance;
    MainThread::sInstance = 0;
}


MainThread * MainThread::sInstance(0);


MainThread & MainThread::Instance()
{
    if (!sInstance)
    {
        throw std::logic_error("MainThread::sInstance == NULL");
    }
    return *sInstance;
}


// This function must be defined in platform-specific code (Qt, Win32, Mac OS X, etc...)
std::auto_ptr<MainThreadImpl> CreateMainThreadImpl();


MainThread::MainThread() :
    mImpl(CreateMainThreadImpl())
{

}


MainThread::~MainThread()
{
}


void MainThread::postAction(Action inAction)
{
    mImpl->postAction(inAction);
}


} // namespace Futile
