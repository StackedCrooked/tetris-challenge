#ifndef AUTOPTRSUPPORT_H
#define AUTOPTRSUPPORT_H


#include <memory>


namespace Futile {


// Helper functions to reduce the typing required to create auto_ptr objects.
// For example:
//
//   std::auto_ptr<Point>(new Point(x, y));
//
// can be written as:
//
//   Create<Point>(x, y);
//
template<class T>
std::auto_ptr<T> Create()
{
    return std::auto_ptr<T>(new T);
}

template<class T, class Arg0>
std::auto_ptr<T> Create(const Arg0& arg0)
{
    return std::auto_ptr<T>(new T(arg0));
}

template<class T, class Arg0, class Arg1>
std::auto_ptr<T> Create(const Arg0& arg0, const Arg1& arg1)
{
    return std::auto_ptr<T>(new T(arg0, arg1));
}

template<class T, class Arg0, class Arg1, class Arg2>
std::auto_ptr<T> Create(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2)
{
    return std::auto_ptr<T>(new T(arg0, arg1, arg2));
}

template<class T, class Arg0, class Arg1, class Arg2, class Arg3>
std::auto_ptr<T> Create(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
{
    return std::auto_ptr<T>(new T(arg0, arg1, arg2, arg3));
}

template<class T, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4>
std::auto_ptr<T> Create(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4)
{
    return std::auto_ptr<T>(new T(arg0, arg1, arg2, arg3, arg4));
}


// Create can be used to create polymorphic instances for std::auto_ptr. For example:
//
//   std::auto_ptr<Shape>(new Triangle(p1, p2, p3));
//
// can be written as:
//
//   CreatePoly<Shape, Triangle>(p1, p2, p3);
//
template<class T, class U>
std::auto_ptr<T> CreatePoly()
{
    return std::auto_ptr<T>(new U);
}

template<class T, class U, class Arg0>
std::auto_ptr<T> CreatePoly(const Arg0& arg0)
{
    return std::auto_ptr<T>(new U(arg0));
}

template<class T, class U, class Arg0, class Arg1>
std::auto_ptr<T> CreatePoly(const Arg0& arg0, const Arg1& arg1)
{
    return std::auto_ptr<T>(new U(arg0, arg1));
}

template<class T, class U, class Arg0, class Arg1, class Arg2>
std::auto_ptr<T> CreatePoly(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2)
{
    return std::auto_ptr<T>(new U(arg0, arg1, arg2));
}

template<class T, class U, class Arg0, class Arg1, class Arg2, class Arg3>
std::auto_ptr<T> CreatePoly(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
{
    return std::auto_ptr<T>(new U(arg0, arg1, arg2, arg3));
}


template<class T, class U, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4>
std::auto_ptr<T> CreatePoly(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4)
{
    return std::auto_ptr<T>(new U(arg0, arg1, arg2, arg3, arg4));
}


} // namespace Futile


#endif // AUTOPTRSUPPORT_H

