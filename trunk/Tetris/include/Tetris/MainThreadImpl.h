#ifndef TETRIS_MAINTHREADIMPL_H_INCLUDED
#define TETRIS_MAINTHREADIMPL_H_INCLUDED


#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <memory>


namespace Tetris {


typedef boost::function<void()> Action;


class MainThreadImpl : boost::noncopyable
{
public:
    virtual ~MainThreadImpl() {}

    virtual void postAction(Action inAction) = 0;
};


} // namespace Tetris


#endif // TETRIS_MAINTHREADIMPL_H_INCLUDED
