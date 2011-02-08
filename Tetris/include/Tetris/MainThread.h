#ifndef TETRIS_MAINTHREAD_H_INCLUDED
#define TETRIS_MAINTHREAD_H_INCLUDED


#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <memory>


namespace Tetris {


typedef boost::function<void()> Action;


/**
 * Invokelater enables worker threads to schedule an action to be executed in the main thread.
 * This is achieved by pushing a (platform specific) "post message" on the main message loop.
 */
void InvokeLater(const Action & inAction);


class MainThreadImpl;


class MainThread
{
public:
    class Initializer
    {
    public:
        Initializer();

        ~Initializer();

    private:
        Initializer(const Initializer&);
        Initializer& operator=(const Initializer&);
    };

    static MainThread & Instance();

    void postAction(Action inAction);

private:
    MainThread(const MainThread&);
    MainThread& operator=(const MainThread);

    MainThread();
    ~MainThread();

    friend class Initializer;

    boost::scoped_ptr<MainThreadImpl> mImpl;

    static MainThread * sInstance;
};


} // namespace Tetris


#endif // TETRIS_MAINTHREAD_H_INCLUDED
