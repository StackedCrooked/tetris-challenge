#ifndef MEMORYPOOL_H_INCLUDED
#define MEMORYPOOL_H_INCLUDED


#include <boost/noncopyable.hpp>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>


namespace Futile {


template<class MemoryPool_>
class ScopedMemoryPoolPtr : boost::noncopyable
{
public:
    typedef MemoryPool_ MemoryPool;
    typedef typename MemoryPool::ValueType ValueType;

    ScopedMemoryPoolPtr(MemoryPool * inPool, ValueType * inValue);

    ~ScopedMemoryPoolPtr();

    void reset();

    const ValueType * get() const { return mValue; }

    ValueType * get() { return mValue; }

    const ValueType * operator->() const { return mValue; }

    ValueType * operator->() { return mValue; }

    const ValueType & operator*() const { return *mValue; }

    ValueType & operator*() { return *mValue; }

private:
    MemoryPool * mPool;
    ValueType * mValue;
};


/**
 * MemoryPool that priorizes fast destruction.
 * Actual memory allocated is 2x the usable size.
 *
 */
template<typename ValueType_>
class MemoryPool : boost::noncopyable
{
public:
    typedef ValueType_ ValueType;
    typedef ScopedMemoryPoolPtr< MemoryPool<ValueType> > ScopedPtr;

    MemoryPool(std::size_t inItemCount) :
        mData(sizeof(ValueType) * inItemCount),
        mItems(),
        mUsedItems(),
        mFreeItems(inItemCount, NULL)
    {
        for (std::size_t idx = 0; idx < inItemCount; ++idx)
        {
            char * data = &mData[idx * sizeof(ValueType)];
            ValueType * value = reinterpret_cast<ValueType*>(data);
            mItems.push_back(Item(value, idx));
            mFreeItems[inItemCount - (idx + 1)] = idx;
        }
    }

    ~MemoryPool()
    {
    }

    /**
     * Returns the max memory size of the pool.
     */
    std::size_t size() const
    {
        return mData.size();
    }

    std::size_t available() const
    {
        return mFreeItems.size();
    }

    std::size_t used() const
    {
        return mUsedItems.size();
    }

    /**
     * Gets a MyClasser to a unconstructed item on the pool.
     *
     * Acquire must be followed by placement new:
     *     MyClass * ptr = new (pool.acquire()) MyClass(3, 4);
     *
     * Relase must be preceded by calling the destructor:
     *     ptr->~MyClass();
     *     pool.release(ptr);
     */
    ValueType * acquire()
    {
        if (mFreeItems.empty())
        {
            throw std::runtime_error("MemoryPool is full.");
        }
        mFreeItems.back();
        mUsedItems.push_back(mFreeItems.back());
        mFreeItems.pop_back();
        return mItems[mUsedItems.back()].mValue;
    }

    ValueType * acquireAndDefaultConstruct()
    {
        return new (acquire()) ValueType();
    }

    /**
     * Acquires a memory slab and constructs the object using the FactoryFunction object.
     * The FactoryFunction signature must be "ValueType* (void*);" or "ValueType* (ValueType*);"
     *
     * Example:
     *
     *   // Point factory function
     *   static Point * CreatePoint(void * placement, int x, int y)
     *   {
     *     return new (placement) Point(x, y);
     *   }
     *
     *   Point * point = pool.acquireAndConstruct(boost::bind(CreatePoint, 3, 4));
     *
     */
    template<class FactoryFunction>
    ValueType * acquireAndConstruct(FactoryFunction inFactoryFunction)
    {
        return inFactoryFunction(acquire());
    }

    /**
     * Returns the MyClasser to the pool.
     *
     * This call must be preceded by calling the destructor:
     *     ptr->~MyClass();
     *     pool.release(ptr);
     */
    void release(const ValueType * inValue)
    {
        for (std::size_t usedIdx = mUsedItems.size() - 1; usedIdx != std::size_t(-1); --usedIdx)
        {
            std::size_t itemIndex = mUsedItems[usedIdx];
            if (mItems[itemIndex].mValue == inValue)
            {
                std::swap(mUsedItems[usedIdx], mUsedItems.back());
                mFreeItems.push_back(mUsedItems.back());
                mUsedItems.pop_back();
                mItems[itemIndex].mUsed = false;
            }
        }
    }

    void destructAndRelease(const ValueType * inValue)
    {
        inValue->~ValueType();
        release(inValue);
    }

    std::size_t indexOf(const ValueType * inValue) const
    {
        return inValue - reinterpret_cast<const ValueType*>(mData.data());
    }

    std::size_t offsetOf(const ValueType * inValue) const
    {
        return indexOf(inValue) * sizeof(ValueType);
    }

private:
    // Contigious container containing all data.
    std::vector<char> mData;

    class Item
    {
    public:
        Item(ValueType * inValue, std::size_t inIndex) :
            mValue(inValue),
            mIndex(inIndex),
            mUsed(false)
        {
        }

        ValueType * mValue;
        std::size_t mIndex;
        bool mUsed;
    };

    std::vector<Item> mItems;

    typedef std::vector<std::size_t> UsedItems;
    UsedItems mUsedItems;

    typedef std::vector<std::size_t> FreeItems;
    FreeItems mFreeItems;
};


template<class MemoryPool>
ScopedMemoryPoolPtr<MemoryPool>::ScopedMemoryPoolPtr(MemoryPool * inPool, ValueType * inValue) :
    mPool(inPool),
    mValue(inValue)
{
}


template<class MemoryPool>
ScopedMemoryPoolPtr<MemoryPool>::~ScopedMemoryPoolPtr()
{
    reset();
}


template<class MemoryPool>
void ScopedMemoryPoolPtr<MemoryPool>::reset()
{
    mValue->~ValueType();
    mPool->release(mValue);
}


} // namespace Futile


#endif // MEMORYPOOL_H_INCLUDED
