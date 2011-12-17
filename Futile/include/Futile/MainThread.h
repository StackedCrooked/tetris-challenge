#ifndef MAINTHREAD_H
#define MAINTHREAD_H


#include "Futile/Singleton.h"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>


namespace Futile {


typedef boost::function<void()> Action;


class MainThreadImpl;


class MainThread : public Singleton<MainThread>
{
public:
    void postAction(Action inAction);

private:
    friend class Singleton<MainThread>;

    MainThread();

    ~MainThread();

    std::unique_ptr<MainThreadImpl> mImpl;
};


/**
 * Invokelater enables worker threads to schedule an action to be executed in the main thread.
 * This is achieved by pushing a (platform specific) "post message" on the main message loop.
 * This can only be used in applications that have a message loop (usually GUI applications).
 */
inline void InvokeLater(const Action & inAction)
{
    MainThread::Instance().postAction(inAction);
}


} // namespace Futile


#endif // MAINTHREAD_H
