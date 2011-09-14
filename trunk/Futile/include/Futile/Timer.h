#ifndef FUTILE_TIMER_H
#define FUTILE_TIMER_H


#include "Futile/Stopwatch.h"
#include "Futile/Threading.h"
#include "Futile/WorkerPool.h"
#include <boost/cstdint.hpp>
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

    Timer(boost::uint64_t inStartInterval,
          boost::uint64_t inPeriodicInterval);

    ~Timer();

    /// Starts the timer.
    /// Throws std::runtime_error if the timer was already started.
    void start(const Action & inAction);

    /// Stops the timer and waits for the current action to finish.
    void stop();

    void setPeriodicInterval(boost::uint64_t inPeriodicInterval);

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Futile


#endif // FUTILE_TIMER_H
