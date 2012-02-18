#ifndef MAINTHREADIMPL_H
#define MAINTHREADIMPL_H


#include <boost/function.hpp>
#include <boost/noncopyable.hpp>


namespace Futile {


typedef boost::function<void()> Action;


/**
 * MainThreadImpl is the base class for platform specific implementations.
 */
class MainThreadImpl : boost::noncopyable
{
public:
    virtual ~MainThreadImpl() {}

#ifdef FUTILE_EVENTLOOP
    virtual void postAction(Action inAction) = 0;
#else
    virtual void postAction(Action) {}
#endif
};


} // namespace Futile


#endif // MAINTHREADIMPL_H
