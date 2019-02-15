#include "Futile/MainThread.h"
#include "Futile/MainThreadImpl.h"
#include <memory>
#include <stdexcept>


namespace Futile {


// This function must be defined in platform-specific code (Qt, Win32, Mac OS X, etc...)
std::unique_ptr<MainThreadImpl> CreateMainThreadImpl();


MainThread::MainThread() :
    mImpl(CreateMainThreadImpl().release())
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
