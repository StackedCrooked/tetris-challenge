#ifndef FUTILE_SINGLETON_H
#define FUTILE_SINGLETON_H


#include "Futile/Assert.h"
#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <typeinfo>


namespace Futile {


template<class T>
class Singleton
{
public:
    typedef Singleton<T> This;

    /// Create an ScopedInitializer object as a named local variable
    /// to have deterministic lifetime of the singleton object.
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


#endif // FUTILE_SINGLETON_H
