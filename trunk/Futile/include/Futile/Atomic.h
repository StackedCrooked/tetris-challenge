#ifndef ATOMIC_H
#define ATOMIC_H


#include "Futile/Threading.h"
#include <boost/function.hpp>


namespace Futile {


template<typename T>
class Atomic : boost::noncopyable
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


template<typename T>
class Synchronized : boost::noncopyable
{
public:
    Synchronized(T inValue = T()) :
        mValue(inValue)
    {
    }

    typedef boost::function<void(const T &)> Reader;

    void apply(Reader reader) const
    {
        ScopedLock lock(mMutex);
        reader(mValue);
    }

    typedef boost::function<void(T &)> Writer;

    void apply(Writer writer)
    {
        ScopedLock lock(mMutex);
        writer(mValue);
    }

private:
    mutable Mutex mMutex;
    T mValue;
};


} // namespace Futile


#endif // ATOMIC_H
