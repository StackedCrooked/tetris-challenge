#ifndef TETRIS_ALLOCATOR_H_INCLUDED
#define TETRIS_ALLOCATOR_H_INCLUDED


#include <cstddef>
#include <vector>


namespace Tetris {


/**
 * Allocator_Vector
 */
template<class T>
class Allocator_Vector
{
public:
    Allocator_Vector(size_t inSize);
    T * alloc();
    void free(T * inBuffer);

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
    T * alloc();
    void free(T * inBuffer);

private:
    Allocator_Malloc(const Allocator_Malloc&);
    Allocator_Malloc& operator=(const Allocator_Malloc&);
    
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
    T * alloc();
    void free(T * inBuffer);

private:
    Allocator_New(const Allocator_New&);
    Allocator_New& operator=(const Allocator_New&);
    
    size_t mSize;
};


/**
 * MemoryPool, helper class for Allocator_Pool
 */
class MemoryPoolImpl;
class MemoryPool
{
public:
    MemoryPool(size_t inItemSize);

    ~MemoryPool();

    void* get();

    void release(void* inData);

private:
    MemoryPool(const MemoryPool&);
    MemoryPool& operator=(const MemoryPool&);

    MemoryPoolImpl * mImpl;
};


/**
 * Allocator_Pool
 */
template<class T>
class Allocator_Pool
{
public:
    Allocator_Pool(size_t inSize);
    T * alloc();
    void free(T * inBuffer);

private:
    Allocator_Pool(const Allocator_Pool&);
    Allocator_Pool& operator=(const Allocator_Pool&);
    
    MemoryPool mMemoryPool;
};


//
// Inlines
//
template<class T>
Allocator_Vector<T>::Allocator_Vector(size_t inSize) :
    mVector(sizeof(T) * inSize)
{
}


template<class T>
T * Allocator_Vector<T>::alloc()
{
	return mVector.data();
}


template<class T>
void Allocator_Vector<T>::free(T * inBuffer)
{
}


template<class T>
Allocator_Malloc<T>::Allocator_Malloc(size_t inSize) :
    mSize(sizeof(T) * inSize)
{
}


template<class T>
T * Allocator_Malloc<T>::alloc()
{
	void * result = malloc(mSize);
	return reinterpret_cast<T*>(result);
}


template<class T>
void Allocator_Malloc<T>::free(T * inBuffer)
{
    ::free(inBuffer);
}


template<class T>
Allocator_New<T>::Allocator_New(size_t inSize) :
    mSize(sizeof(T) * inSize)
{
}


template<class T>
T * Allocator_New<T>::alloc()
{
    return new T[mSize];
}


template<class T>
void Allocator_New<T>::free(T * inBuffer)
{
    delete [] inBuffer;
}


template<class T>
Allocator_Pool<T>::Allocator_Pool(size_t inSize) :
    mMemoryPool(sizeof(T) * inSize)
{
}


template<class T>
T * Allocator_Pool<T>::alloc()
{
    return reinterpret_cast<T*>(mMemoryPool.get());
}


template<class T>
void Allocator_Pool<T>::free(T * inBuffer)
{
    mMemoryPool.release(inBuffer);
}


} // namespace Tetris

#endif // TETRIS_ALLOCATOR_H_INCLUDED

