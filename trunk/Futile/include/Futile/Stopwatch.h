#ifndef FUTILE_STOPWATCH_H
#define FUTILE_STOPWATCH_H


#include <boost/noncopyable.hpp>
#include <ctime>


namespace Futile {


class Stopwatch : boost::noncopyable
{
public:
    /// Creates and starts the stopwatch.
    Stopwatch();

    /// Resets and starts the stopwatch.
    void restart();

    /// Returns the elapsed time in milliseconds.
    unsigned elapsedMs() const;

private:
    clock_t mStart;
};


} // namespace Futile


#endif // FUTILE_STOPWATCH_H
