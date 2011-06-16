#ifndef MEMORYPOOL_H_INCLUDED
#define MEMORYPOOL_H_INCLUDED


#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>


namespace Futile {
namespace Memory {
namespace Pool {


class MemoryPool;


struct PtrBase
{
    PtrBase(std::size_t inTypeSize) :
        mTypeSize(inTypeSize)
    {
    }

    ~PtrBase() {}

    std::size_t size() const { return mTypeSize; }
    
    std::size_t mTypeSize;
};


/**
 * Ptr encapsulates a pointer value and provides various ways to access it.
 */
template<class T>
struct Ptr : public PtrBase
{
    typedef T Type;

    Ptr(MemoryPool * inPool, Type * inValue) :
        PtrBase(sizeof(Type)),
        mData(new Data(inPool, inValue))
    {
    }

    Ptr(const Ptr<T> & rhs) :
        PtrBase(sizeof(Type)),
        mData(rhs.mData)
    {
        ++mData->mRefCount;
    }

    Ptr<T> & operator=(const Ptr<T> & rhs)
    {
        Ptr<T> copy(rhs);
        std::swap(mData, copy.mData);
        return *this;
    }

    ~Ptr()
    {
        reset();
    }

    void reset()
    {
        if (--mData->mRefCount == 0)
        {
            MemoryPool & pool = *mData->mPool;
            while (this != pool.top())
            {
                // Pop all higher variables.
                pool.pop();
            }
            pool.pop();
        }
    }

    inline const Type * get() const { return mValue; }

    inline Type * get() { return mValue; }

    inline const Type * operator->() const { return get(); }

    inline Type * operator->() { return get(); }

    inline const Type & operator*() const { return *get(); }

    inline Type & operator*() { return *get(); }

private:
    struct Data
    {
        Data(MemoryPool * inPool, Type * inValue) :
            mPool(inPool),
            mValue(inValue),
            mRefCount(1)
        {
        }

        MemoryPool * mPool;
        Type * mValue;
        unsigned mRefCount;
    };
    Data * mData;
};


/**
 * MemoryPool that priorizes fast destruction.
 */
class MemoryPool : boost::noncopyable
{
public:
    MemoryPool(std::size_t inSize) :
        mData(inSize, 0)
    {
    }

    ~MemoryPool()
    {
    }

    template<class T>
    Ptr<T> create()
    {
        Ptr<T> result(this, new (&mData[0] + mOffset) T);
        mOffset += sizeof(T);
        mStack.push_back(&result);
        return result;
    }

    PtrBase * top()
    {
        return mStack.back();
    }

    void pop()
    {
        PtrBase * backPtr = mStack.back();
        mOffset -= backPtr->size();
        delete backPtr;
        mStack.pop_back();
    }

    // Contigious container containing all data.
    std::vector<char> mData;
    std::vector<char>::size_type mOffset;
    std::vector<PtrBase*> mStack;
};


} } } // namespace Futile::Memory::Pool


#endif // MEMORYPOOL_H_INCLUDED
