#ifndef SINGLETON_H
#define SINGLETON_H


#include "Futile/Assert.h"
#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <typeinfo>


namespace Futile {


/**
 * This singleton implementation requires the user to declare a ScopedInitializer object
 * to create the actual instance and define the lifetime scope of the object. This can
 * be done at the beginning of the application's main function.
 *
 * This approach sacrifices lazy instantiation in favor of deterministic lifetime of the
 * singleton object. I think this design is more congruent with the C++ way of doing
 * things.
 */
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

    static T& Instance()
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
