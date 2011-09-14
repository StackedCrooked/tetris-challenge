#ifndef FUTILE_TIMER_H
#define FUTILE_TIMER_H


#include "Futile/Stopwatch.h"
#include "Futile/Threading.h"
#include "Futile/WorkerPool.h"
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>


namespace Futile {


/**
 * Threaded timer class
 */
class Timer : boost::noncopyable
{
public:
    typedef boost::function<void()> Action;

    Timer(const Action & inAction,
          unsigned inStartInterval,
          unsigned inPeriodicInterval);

    ~Timer();

    /// Starts the timer.
    /// Throws std::runtime_error if the timer was already started.
    void start();

    /// Stops the timer and waits for the current action to finish.
    void stop();

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Futile


#endif // FUTILE_TIMER_H
