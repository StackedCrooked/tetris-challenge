#ifndef FUTILE_STOPWATCH_H
#define FUTILE_STOPWATCH_H


#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <ctime>


namespace Futile {


class Stopwatch : boost::noncopyable
{
public:
    /// Creates and starts the stopwatch.
    Stopwatch();

    /// Reset and start the stopwatch.
    void restart();

    /// Returns the elapsed time in milliseconds.
    boost::uint64_t elapsedMs() const;

private:
    boost::uint64_t mStart;
};


} // namespace Futile


#endif // FUTILE_STOPWATCH_H
