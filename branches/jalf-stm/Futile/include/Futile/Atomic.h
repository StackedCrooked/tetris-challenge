#ifndef ATOMIC_H
#define ATOMIC_H


#include "Futile/Threading.h"
#include <boost/function.hpp>


namespace Futile {


template<typename T>
class Atomic
{
public:
    typedef Atomic<T> This;

    Atomic(T inValue = T()) :
        mValue(inValue)
    {
    }

    Atomic(const This & rhs) :
        mMutex(),
        mValue(rhs.get())
    {
    }

    Atomic & operator=(const This & rhs)
    {
        set(rhs.get());
        return *this;
    }

    ~Atomic()
    {
        // No implementation required.
    }

    // Get a copy of the value
    T get() const
    {
        ScopedLock lock(mMutex);
        return mValue;
    }

    void set(const T & inValue)
    {
        ScopedLock lock(mMutex);
        mValue = inValue;
    }

    void increment(const T & inValue)
    {
        ScopedLock lock(mMutex);
        mValue += inValue;
    }

    void decrement(const T & inValue)
    {
        ScopedLock lock(mMutex);
        mValue -= inValue;
    }

private:
    mutable Mutex mMutex;
    T mValue;
};


} // namespace Futile


#endif // ATOMIC_H
