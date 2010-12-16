#ifndef TETRIS_ALLOCATOR_H_INCLUDED
#define TETRIS_ALLOCATOR_H_INCLUDED


#include <cstddef>
#include <vector>


namespace Tetris {


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

    T * get();

    const T * get() const;

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

	~Allocator_Malloc();

    T * get();

    const T * get() const;

private:
    Allocator_Malloc(const Allocator_Malloc&);
    Allocator_Malloc& operator=(const Allocator_Malloc&);
    
    T * mBuffer;
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

    T * get();

    const T * get() const;

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
T * Allocator_Vector<T>::get()
{
	return &mVector[0];
}


template<class T>
const T * Allocator_Vector<T>::get() const
{
	return &mVector[0];
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(size_t inSize) :
    mBuffer(malloc(sizeof(T) * inSize))
{
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(size_t inSize, const T & inInitialValue) :
    mBuffer(reinterpret_cast<T*>(malloc(sizeof(T) * inSize)))
{
	Allocator_FillBuffer(mBuffer, inSize, inInitialValue);
}


template<class T>
Allocator_Malloc<T>::~Allocator_Malloc()
{
	free(mBuffer);
}


template<class T>
T * Allocator_Malloc<T>::get()
{
	return mBuffer;
}


template<class T>
const T * Allocator_Malloc<T>::get() const
{
	return mBuffer;
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
T * Allocator_New<T>::get()
{
    return &mBuffer[0];
}


template<class T>
const T * Allocator_New<T>::get() const
{
    return &mBuffer[0];
}


} // namespace Tetris


#endif // TETRIS_ALLOCATOR_H_INCLUDED
