#ifndef ATOMIC_H
#define ATOMIC_H


#include "Futile/Threading.h"


namespace Futile {


template<typename T>
class Atomic
{
public:
    Atomic(T inValue = T()) :
        mValue(inValue)
    {
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

private:
    mutable Mutex mMutex;
    T mValue;
};


} // namespace Futile


#endif // ATOMIC_H
