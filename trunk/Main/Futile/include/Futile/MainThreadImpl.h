#ifndef FUTILE_MAINTHREADIMPL_H_INCLUDED
#define FUTILE_MAINTHREADIMPL_H_INCLUDED


#include <boost/function.hpp>
#include <boost/noncopyable.hpp>


namespace Futile {


typedef boost::function<void()> Action;


class MainThreadImpl : boost::noncopyable
{
public:
    virtual ~MainThreadImpl() {}

    virtual void postAction(Action inAction) = 0;
};


} // namespace Futile


#endif // FUTILE_MAINTHREADIMPL_H_INCLUDED
