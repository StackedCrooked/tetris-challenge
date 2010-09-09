#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED


#include <memory>


namespace Tetris
{    


    // Rounded division by 2.
    // To be used for integral values (int, size_t, ...).
    template<class T>
    T DivideByTwo(T inValue)
    {
        return (int)(0.5 + ((float)inValue / (float)2.0));
    }


    // Helper functions to reduce the typing involed when creating auto_ptr objects.
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
    std::auto_ptr<T> Create(Arg0 arg0)
    {
        return std::auto_ptr<T>(new T(arg0));
    }

    template<class T, class Arg0, class Arg1>
    std::auto_ptr<T> Create(Arg0 arg0, Arg1 arg1)
    {
        return std::auto_ptr<T>(new T(arg0, arg1));
    }

    template<class T, class Arg0, class Arg1, class Arg2>
    std::auto_ptr<T> Create(Arg0 arg0, Arg1 arg1, Arg2 arg2)
    {
        return std::auto_ptr<T>(new T(arg0, arg1, arg2));
    }

    template<class T, class Arg0, class Arg1, class Arg2, class Arg3>
    std::auto_ptr<T> Create(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        return std::auto_ptr<T>(new T(arg0, arg1, arg2, arg3));
    }


    // Create can be used to create polymorphic instances for std::auto_ptr. For example:
    //
    //   std::auto_ptr<Shape>(new Triangle(p1, p2, p3));
    //
    // can be written as:
    //
    //   Create<Shape, Triangle>(p1, p2, p3);
    //
    template<class T, class U>
    std::auto_ptr<T> CreatePoly()
    {
        return std::auto_ptr<T>(new U);
    }

    template<class T, class U, class Arg0>
    std::auto_ptr<T> CreatePoly(Arg0 arg0)
    {
        return std::auto_ptr<T>(new U(arg0));
    }

    template<class T, class U, class Arg0, class Arg1>
    std::auto_ptr<T> CreatePoly(Arg0 arg0, Arg1 arg1)
    {
        return std::auto_ptr<T>(new U(arg0, arg1));
    }

    template<class T, class U, class Arg0, class Arg1, class Arg2>
    std::auto_ptr<T> CreatePoly(Arg0 arg0, Arg1 arg1, Arg2 arg2)
    {
        return std::auto_ptr<T>(new U(arg0, arg1, arg2));
    }

    template<class T, class U, class Arg0, class Arg1, class Arg2, class Arg3>
    std::auto_ptr<T> CreatePoly(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        return std::auto_ptr<T>(new U(arg0, arg1, arg2, arg3));
    }


} // namespace Tetris


#endif // UTILITIES_H_INCLUDED
