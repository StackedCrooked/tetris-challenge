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

    /// Create an Initializer object as a named local variable
    /// to have deterministic lifetime of the singleton object.
    struct Initializer : boost::noncopyable
    {
        Initializer() { This::CreateInstance(); }

        ~Initializer() { This::DestroyInstance(); }
    };

    static T & Instance()
    {
        if (!sInstance)
        {
			throw std::logic_error("No instance. Did you forget to create a named Singleton<T>::Initializer object?");
        }
        return *sInstance;
    }

protected:
    friend struct This::Initializer;

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
