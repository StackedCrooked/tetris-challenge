#ifndef AUTOPTRSUPPORT_H_INCLUDED
#define AUTOPTRSUPPORT_H_INCLUDED


#include <memory>


namespace Futile {


// Helper functions to reduce the typing involed when creating unique_ptr objects.
// For example:
//
//   std::unique_ptr<Point>(new Point(x, y));
//
// can be written as:
//
//   Create<Point>(x, y);
//
template<class T>
std::unique_ptr<T> Create()
{
    return std::unique_ptr<T>(new T);
}

template<class T, class Arg0>
std::unique_ptr<T> Create(Arg0 arg0)
{
    return std::unique_ptr<T>(new T(arg0));
}

template<class T, class Arg0, class Arg1>
std::unique_ptr<T> Create(Arg0 arg0, Arg1 arg1)
{
    return std::unique_ptr<T>(new T(arg0, arg1));
}

template<class T, class Arg0, class Arg1, class Arg2>
std::unique_ptr<T> Create(Arg0 arg0, Arg1 arg1, Arg2 arg2)
{
    return std::unique_ptr<T>(new T(arg0, arg1, arg2));
}

template<class T, class Arg0, class Arg1, class Arg2, class Arg3>
std::unique_ptr<T> Create(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
    return std::unique_ptr<T>(new T(arg0, arg1, arg2, arg3));
}

template<class T, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4>
std::unique_ptr<T> Create(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{
    return std::unique_ptr<T>(new T(arg0, arg1, arg2, arg3, arg4));
}


// Create can be used to create polymorphic instances for std::unique_ptr. For example:
//
//   std::unique_ptr<Shape>(new Triangle(p1, p2, p3));
//
// can be written as:
//
//   Create<Shape, Triangle>(p1, p2, p3);
//
template<class T, class U>
std::unique_ptr<T> CreatePoly()
{
    return std::unique_ptr<T>(new U);
}

template<class T, class U, class Arg0>
std::unique_ptr<T> CreatePoly(Arg0 arg0)
{
    return std::unique_ptr<T>(new U(arg0));
}

template<class T, class U, class Arg0, class Arg1>
std::unique_ptr<T> CreatePoly(Arg0 arg0, Arg1 arg1)
{
    return std::unique_ptr<T>(new U(arg0, arg1));
}

template<class T, class U, class Arg0, class Arg1, class Arg2>
std::unique_ptr<T> CreatePoly(Arg0 arg0, Arg1 arg1, Arg2 arg2)
{
    return std::unique_ptr<T>(new U(arg0, arg1, arg2));
}

template<class T, class U, class Arg0, class Arg1, class Arg2, class Arg3>
std::unique_ptr<T> CreatePoly(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
    return std::unique_ptr<T>(new U(arg0, arg1, arg2, arg3));
}


template<class T, class U, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4>
std::unique_ptr<T> CreatePoly(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{
    return std::unique_ptr<T>(new U(arg0, arg1, arg2, arg3, arg4));
}


} // namespace Futile


#endif // AUTOPTRSUPPORT_H_INCLUDED

