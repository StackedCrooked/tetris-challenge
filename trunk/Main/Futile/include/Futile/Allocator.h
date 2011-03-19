#ifndef TETRIS_ALLOCATOR_H_INCLUDED
#define TETRIS_ALLOCATOR_H_INCLUDED


#include "Futile/Assert.h"
#include <cstddef>
#include <cstdlib>
#include <vector>


namespace Futile {


template<class T>
inline void Allocator_FillBuffer(T * inBuffer, size_t inSize, const T & inInitialValue)
{
    for (size_t i = 0; i < inSize; ++i)
    {
        inBuffer[i] = inInitialValue;
    }
}


/**
 * Allocator_Vector
 */
template<class T>
class Allocator_Vector
{
public:
    Allocator_Vector(size_t inSize);

    Allocator_Vector(size_t inSize, const T & inInitialValue);

    T & get(size_t inIndex);

    const T & get(size_t inIndex) const;

    void set(size_t inIndex, const T & inValue);

private:
    std::vector<T> mVector;
};


/**
 * Allocator_Malloc
 */
template<class T>
class Allocator_Malloc
{
public:
    Allocator_Malloc(size_t inSize);

    Allocator_Malloc(size_t inSize, const T & inInitialValue);

    Allocator_Malloc(const Allocator_Malloc&);

    Allocator_Malloc& operator=(const Allocator_Malloc&);

    ~Allocator_Malloc();

    T & get(size_t inIndex);

    const T & get(size_t inIndex) const;

    void set(size_t inIndex, const T & inValue);

private:
    T * mBuffer;
    size_t mSize;
};


/**
 * Allocator_New
 */
template<class T>
class Allocator_New
{
public:
    Allocator_New(size_t inSize);

    Allocator_New(size_t inSize, const T & inInitialValue);

    ~Allocator_New();

    T & get(size_t inIndex);

    const T & get(size_t inIndex) const;

    void set(size_t inIndex, const T & inValue);

private:
    Allocator_New(const Allocator_New&);
    Allocator_New& operator=(const Allocator_New&);

    T * mBuffer;
};


//
// Inlines
//
template<class T>
Allocator_Vector<T>::Allocator_Vector(size_t inSize) :
    mVector(inSize)
{
}
template<class T>
Allocator_Vector<T>::Allocator_Vector(size_t inSize, const T & inInitialValue) :
    mVector(inSize, inInitialValue)
{
}


template<class T>
T & Allocator_Vector<T>::get(size_t inIndex)
{
    return mVector[inIndex];
}


template<class T>
const T & Allocator_Vector<T>::get(size_t inIndex) const
{
    return mVector[inIndex];
}


template<class T>
void Allocator_Vector<T>::set(size_t inIndex, const T & inValue)
{
    Assert(inIndex <= inIndex);
    mVector[inIndex] = inValue;
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(size_t inSize) :
    mBuffer(reinterpret_cast<T*>(malloc(sizeof(T) * inSize))),
    mSize(inSize)
{
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(size_t inSize, const T & inInitialValue) :
    mBuffer(reinterpret_cast<T*>(malloc(sizeof(T) * inSize))),
    mSize(inSize)
{
    Allocator_FillBuffer(mBuffer, inSize, inInitialValue);
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(const Allocator_Malloc & rhs) :
    mBuffer(reinterpret_cast<T*>(malloc(sizeof(T) * rhs.mSize))),
    mSize(rhs.mSize)
{
    std::copy(rhs.mBuffer, rhs.mBuffer + rhs.mSize, mBuffer);
}


template<class T>
Allocator_Malloc<T> & Allocator_Malloc<T>::operator=(const Allocator_Malloc & rhs)
{
    if (mBuffer != rhs.mBuffer)
    {
        free(mBuffer);
        mBuffer = reinterpret_cast<T*>(malloc(sizeof(T) * rhs.mSize));
        mSize = rhs.mSize;
        std::copy(rhs.mBuffer, rhs.mBuffer + rhs.mSize, mBuffer);
    }
    return *this;
}


template<class T>
Allocator_Malloc<T>::~Allocator_Malloc()
{
    free(mBuffer);
}


template<class T>
T & Allocator_Malloc<T>::get(size_t inIndex)
{
    Assert(inIndex <= mSize);
    return mBuffer[inIndex];
}


template<class T>
const T & Allocator_Malloc<T>::get(size_t inIndex) const
{
    Assert(inIndex <= inIndex);
    return mBuffer[inIndex];
}


template<class T>
void Allocator_Malloc<T>::set(size_t inIndex, const T & inValue)
{
    Assert(inIndex <= inIndex);
    mBuffer[inIndex] = inValue;
}


template<class T>
Allocator_New<T>::Allocator_New(size_t inSize) :
    mBuffer(new T[inSize])
{
}


template<class T>
Allocator_New<T>::Allocator_New(size_t inSize, const T & inInitialValue) :
    mBuffer(new T[inSize])
{
    Allocator_FillBuffer(mBuffer, inSize, inInitialValue);
}


template<class T>
Allocator_New<T>::~Allocator_New()
{
    delete [] mBuffer;
}


template<class T>
T & Allocator_New<T>::get(size_t inIndex)
{
    return mBuffer[inIndex];
}


template<class T>
const T & Allocator_New<T>::get(size_t inIndex) const
{
    return mBuffer[inIndex];
}


template<class T>
void Allocator_New<T>::set(size_t inIndex, const T & inValue)
{
    Assert(inIndex <= inIndex);
    mBuffer[inIndex] = inValue;
}


} // namespace Futile


#endif // TETRIS_ALLOCATOR_H_INCLUDED
