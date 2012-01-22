#ifndef FUTILE_STOPWATCH_H
#define FUTILE_STOPWATCH_H


#include "Futile/Types.h"
#include <boost/noncopyable.hpp>
#include <ctime>


namespace Futile {


class Stopwatch : boost::noncopyable
{
public:
    /// Creates and starts the stopwatch.
    Stopwatch();

    void start();

    void stop();

    /// Reset and start the stopwatch.
    void restart();

    /// Resets the stopwatch
    void reset();

    /// Returns the elapsed time in milliseconds.
    UInt64 elapsedMs() const;

private:
    UInt64 mStart;
    UInt64 mStop;
};


} // namespace Futile


#endif // FUTILE_STOPWATCH_H
