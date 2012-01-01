#ifndef FUTILE_TIMER_H
#define FUTILE_TIMER_H


#include "Futile/Stopwatch.h"
#include "Futile/WorkerPool.h"
#include "Futile/Threading.h"
#include "Futile/Types.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <functional>


namespace Futile {


/**
 * Threaded timer class
 */
class Timer : boost::noncopyable
{
public:
    typedef std::function<void()> Action;

    Timer(UInt64 inInterval);

    ~Timer();

    /// Starts the timer.
    /// Throws std::runtime_error if the timer was already started.
    void start(const Action & inAction);

    /// Stops the timer and waits for the current action to finish.
    void stop();

    /// Sets the timer interval.
    void setInterval(UInt64 inInterval);

    UInt64 getInterval() const;

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Futile


#endif // FUTILE_TIMER_H
