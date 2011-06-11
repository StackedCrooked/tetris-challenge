#include "Futile/Config.h"
#include "Futile/Threading.h"
#include <ctime>


namespace Futile {


struct Stopwatch::Impl : boost::noncopyable
{
    Impl() :
        mStart(clock())
    {

    }

    ~Impl()
    {
    }

    clock_t mStart;
};


Stopwatch::Stopwatch() :
    mImpl(new Impl)
{
}


Stopwatch::~Stopwatch()
{
    mImpl.reset();
}


unsigned Stopwatch::elapsedTimeMs() const
{
    static const clock_t CLOCKS_PER_MS = CLOCKS_PER_SEC / 1000;
    return static_cast<int>((clock() - mImpl->mStart) / CLOCKS_PER_MS);
}


} // namespace Futile
