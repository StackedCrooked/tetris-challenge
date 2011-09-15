#ifndef SINGLETON_H
#define SINGLETON_H


#include "Futile/Assert.h"
#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <typeinfo>


namespace Futile {


/**
 * Singleton implementation with deterministic lifetime and non-lazy instantiation.
 */
template<class T>
class Singleton
{
public:
    typedef Singleton<T> This;

    /// Create an ScopedInitializer object somewhere
    /// in the beginning of your program to create
    /// the singleton object.
    ///
    /// The scope of this object defines the lifetime
    /// of the singleton instance.
    struct ScopedInitializer : boost::noncopyable
    {
        ScopedInitializer() { This::CreateInstance(); }

        ~ScopedInitializer() { This::DestroyInstance(); }
    };

    static T & Instance()
    {
        Assert(sInstance);
        return *sInstance;
    }

protected:
    friend struct This::ScopedInitializer;

    Singleton() { }

    ~Singleton() { }

private:
    static void CreateInstance()
    {
        sInstance = new T;
    }

    static void DestroyInstance()
    {
        delete sInstance;
        sInstance = 0;
    }

    static T * sInstance;
};


template<class T>
T * Singleton<T>::sInstance(0);


} // namespace Futile


#endif // SINGLETON_H
