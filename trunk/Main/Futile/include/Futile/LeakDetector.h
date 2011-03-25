#ifndef FUTILE_LEAKDETECTOR_H
#define FUTILE_LEAKDETECTOR_H


#include "Futile/Singleton.h"
#include <map>
#include <stdexcept>
#include <typeinfo>


namespace Futile {


template<class> class CountedInstance;


/**
 * LeakDetector maintains a list of active instances.
 * Any instances that still exist in this list during
 * destruction of the LeakDetector Singleton object
 * will be reported as memory leaks.
 */
class LeakDetector : public Singleton<LeakDetector>
{
public:
    template<class T>
    void insert(CountedInstance<T> * inObject)
    {
        mTypeInfos.insert(std::make_pair(inObject, &typeid(T)));
    }

    template<class T>
    void erase(CountedInstance<T> * inObject)
    {
        TypeInfos::iterator it = mTypeInfos.find(inObject);
        if (it == mTypeInfos.end())
        {
            throw std::logic_error("LeakDetector::erase: Object not found!");
        }
        mTypeInfos.erase(it);
    }

protected:
    LeakDetector();

    ~LeakDetector();

private:
    typedef Singleton<LeakDetector> Super;
    friend class Singleton<LeakDetector>;

    typedef std::map<void *, const std::type_info * > TypeInfos;
    TypeInfos mTypeInfos;
};


/**
 * A class may inherit CountedInstance<ClassName> to enable leak detection.
 */
template<class SubType>
class CountedInstance : boost::noncopyable
{
public:
    CountedInstance()
    {
        LeakDetector::Instance().insert<SubType>(this);
    }

    ~CountedInstance()
    {
        LeakDetector::Instance().erase<SubType>(this);
    }
};


} // namespace Futile


#endif // FUTILE_LEAKDETECTOR_H
