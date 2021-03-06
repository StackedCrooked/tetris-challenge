#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED


#include "Futile/Assert.h"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <vector>


namespace Futile {


/**
 * Allocator_Vector manages memory through a std::vector<T> object
 */
template<class T>
class Allocator_Vector
{
public:
    Allocator_Vector();

    Allocator_Vector(std::size_t inSize);

    Allocator_Vector(std::size_t inSize, const T& inInitialValue);

    T& get(std::size_t inIndex);

    const T& get(std::size_t inIndex) const;

    void set(std::size_t inIndex, const T& inValue);

    typename std::vector<T>::size_type size() const;

private:
    std::vector<T> mVector;
};


/**
 * Allocator_Malloc manages memory with malloc/free.
 */
template<class T>
class Allocator_Malloc
{
public:
    Allocator_Malloc();

    Allocator_Malloc(std::size_t inSize);

    Allocator_Malloc(std::size_t inSize, const T& inInitialValue);

    Allocator_Malloc(const Allocator_Malloc&);

    Allocator_Malloc& operator=(Allocator_Malloc rhs); // rhs by value! (copy& swap idiom)

    ~Allocator_Malloc();

    void swap(Allocator_Malloc<T>& rhs);

    T& get(std::size_t inIndex);

    const T& get(std::size_t inIndex) const;

    void set(std::size_t inIndex, const T& inValue);

    std::size_t size() const;

private:
    T* mBuffer;
    std::size_t mSize;
};


/**
 * Allocator_New manages memory with new[]/delete[].
 */
template<class T>
class Allocator_New
{
public:
    Allocator_New();

    Allocator_New(std::size_t inSize);

    Allocator_New(std::size_t inSize, const T& inInitialValue);

    Allocator_New(const Allocator_New& rhs);

    Allocator_New& operator=(Allocator_New rhs); // rhs by value! (copy& swap idiom)

    ~Allocator_New();

    void swap(Allocator_New<T>& rhs);

    T& get(std::size_t inIndex);

    const T& get(std::size_t inIndex) const;

    void set(std::size_t inIndex, const T& inValue);

    std::size_t size() const;

private:
    T* mBuffer;
    std::size_t mSize;
};


//
// Inlines
//
template<class T>
Allocator_Vector<T>::Allocator_Vector() :
    mVector()
{
}


template<class T>
Allocator_Vector<T>::Allocator_Vector(std::size_t inSize) :
    mVector(inSize)
{
}


template<class T>
Allocator_Vector<T>::Allocator_Vector(std::size_t inSize, const T& inInitialValue) :
    mVector(inSize, inInitialValue)
{
}


template<class T>
T& Allocator_Vector<T>::get(std::size_t inIndex)
{
    return mVector[inIndex];
}


template<class T>
const T& Allocator_Vector<T>::get(std::size_t inIndex) const
{
    return mVector[inIndex];
}


template<class T>
void Allocator_Vector<T>::set(std::size_t inIndex, const T& inValue)
{
    Assert(inIndex <= size());
    mVector[inIndex] = inValue;
}


template<class T>
typename std::vector<T>::size_type Allocator_Vector<T>::size() const
{
    return mVector.size();
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc() :
    mBuffer(0),
    mSize(0)
{
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(std::size_t inSize) :
    mBuffer(reinterpret_cast<T*>(malloc(sizeof(T) * inSize))),
    mSize(inSize)
{
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(std::size_t inSize, const T& inInitialValue) :
    mBuffer(reinterpret_cast<T*>(malloc(sizeof(T) * inSize))),
    mSize(inSize)
{
    std::fill(mBuffer, mBuffer + inSize, inInitialValue);
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(const Allocator_Malloc& rhs) :
    mBuffer(reinterpret_cast<T*>(malloc(sizeof(T) * rhs.mSize))),
    mSize(rhs.mSize)
{
    std::copy(rhs.mBuffer, rhs.mBuffer + rhs.mSize, mBuffer);
}


template<class T>
Allocator_Malloc<T>& Allocator_Malloc<T>::operator=(Allocator_Malloc rhs)
{
    swap(rhs);
    return *this;
}


template<class T>
Allocator_Malloc<T>::~Allocator_Malloc()
{
    free(mBuffer);
}


template<class T>
void Allocator_Malloc<T>::swap(Allocator_Malloc<T>& rhs)
{
    std::swap(mBuffer, rhs.mBuffer);
    std::swap(mSize, rhs.mSize);
}


template<class T>
T& Allocator_Malloc<T>::get(std::size_t inIndex)
{
    Assert(inIndex <= size());
    return mBuffer[inIndex];
}


template<class T>
const T& Allocator_Malloc<T>::get(std::size_t inIndex) const
{
    Assert(inIndex <= size());
    return mBuffer[inIndex];
}


template<class T>
void Allocator_Malloc<T>::set(std::size_t inIndex, const T& inValue)
{
    Assert(inIndex <= size());
    mBuffer[inIndex] = inValue;
}


template<class T>
std::size_t Allocator_Malloc<T>::size() const
{
    return mSize;
}


template<class T>
Allocator_New<T>::Allocator_New() :
    mBuffer(0),
    mSize(0)
{
}


template<class T>
Allocator_New<T>::Allocator_New(std::size_t inSize) :
    mBuffer(new T[inSize]),
    mSize(inSize)
{
}


template<class T>
Allocator_New<T>::Allocator_New(std::size_t inSize, const T& inInitialValue) :
    mBuffer(new T[inSize]),
    mSize(inSize)
{
    std::fill(mBuffer, mBuffer + mSize, inInitialValue);
}


template<class T>
Allocator_New<T>::Allocator_New(const Allocator_New& rhs) :
    mBuffer(new T[rhs.mSize]),
    mSize(rhs.mSize)
{
    std::copy(rhs.mBuffer, rhs.mBuffer + rhs.mSize, mBuffer);
}


template<class T>
Allocator_New<T>& Allocator_New<T>::operator=(Allocator_New<T> rhs)
{
    swap(rhs);
    return *this;
}


template<class T>
void Allocator_New<T>::swap(Allocator_New<T>& rhs)
{
    std::swap(mBuffer, rhs.mBuffer);
    std::swap(mSize, rhs.mSize);
}


template<class T>
Allocator_New<T>::~Allocator_New()
{
    delete [] mBuffer;
}


template<class T>
T& Allocator_New<T>::get(std::size_t inIndex)
{
    return mBuffer[inIndex];
}


template<class T>
const T& Allocator_New<T>::get(std::size_t inIndex) const
{
    return mBuffer[inIndex];
}


template<class T>
void Allocator_New<T>::set(std::size_t inIndex, const T& inValue)
{
    Assert(inIndex <= size());
    mBuffer[inIndex] = inValue;
}


template<class T>
std::size_t Allocator_New<T>::size() const
{
    return mSize;
}


} // namespace Futile


#endif // ALLOCATOR_H_INCLUDED
