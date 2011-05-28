#ifndef MemoryPoolTypeH_INCLUDED
#define MemoryPoolTypeH_INCLUDED


#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>


namespace Futile {


template<class MemoryPoolType>
struct BasicOwnershipStrategy
{
    typedef typename MemoryPoolType::MemoryPool MemoryPool;
    typedef typename MemoryPoolType::Value Value;
    typedef BasicOwnershipStrategy<MemoryPool> This;

    BasicOwnershipStrategy(MemoryPool & inMemoryPool) :
        mMemoryPool(&inMemoryPool),
        mValue(NULL)
    {
    }

    BasicOwnershipStrategy(MemoryPool & inMemoryPool, Value * inValue) :
        mMemoryPool(&inMemoryPool),
        mValue(inValue)
    {
    }

    BasicOwnershipStrategy(const This & rhs) :
        mMemoryPool(rhs.mMemoryPool),
        mValue(rhs.mValue)
    {
    }

    BasicOwnershipStrategy & operator=(This rhs)
    {
        This::swap(rhs);
        return *this;
    }

    ~BasicOwnershipStrategy() { }

    const Value * get() const { return mValue; }

    Value * get() { return mValue; }

    void reset(Value * inValue)
    {
        destroy();
        mValue = inValue;
    }

private:
    void swap(This & rhs)
    {
        std::swap(mMemoryPool, rhs.mMemoryPool);
        std::swap(mValue, rhs.mValue);
    }

    inline void destroy()
    {
        if (mValue)
        {
            mValue->~Value();
        }
        mMemoryPool->release(mValue);
    }

    MemoryPool * mMemoryPool;
    Value * mValue;
};


template<class MemoryPoolType, class ValueType, class OwnershipStrategyType>
class MemoryPool_SmartPointer : public OwnershipStrategyType
{
public:
    typedef MemoryPoolType MemoryPool;
    typedef ValueType Value;
    typedef OwnershipStrategyType OwnershipStrategy;
    typedef OwnershipStrategy Base;
    typedef MemoryPool_SmartPointer<MemoryPool, Value, Base> This;

    MemoryPool_SmartPointer(MemoryPool & inMemoryPool) :
        OwnershipStrategy(inMemoryPool)
    {
    }

    MemoryPool_SmartPointer(MemoryPool & inMemoryPool, Value * inValue) :
        OwnershipStrategy(inMemoryPool, inValue)
    {
    }

    MemoryPool_SmartPointer(const This & rhs) :
        Base(rhs)
    {
    }

    This & operator=(This rhs)
    {
        Base::swap(rhs);
        return *this;
    }

    ~MemoryPool_SmartPointer() { }

    const Value * operator->() const { return OwnershipStrategy::get(); }

    Value * operator->() { return OwnershipStrategy::get(); }

    const Value & operator*() const { return *OwnershipStrategy::get(); }

    Value & operator*() { return *OwnershipStrategy::get(); }

private:
    void destroy()
    {
        Value * value = OwnershipStrategy::get();
        value->~Value();
        OwnershipStrategy::mMemoryPool->release(value);
    }
};


/**
 * SharedOwnershipStrategy for SharedPtr class.
 */
template<class MemoryPoolType>
struct SharedOwnershipStrategy : public BasicOwnershipStrategy<MemoryPoolType>
{
    typedef BasicOwnershipStrategy<MemoryPoolType> Base;
    typedef SharedOwnershipStrategy<MemoryPoolType> This;

    typedef MemoryPoolType MemoryPool;
    typedef typename MemoryPool::Value Value;

    SharedOwnershipStrategy(MemoryPool & inMemoryPool) :
        Base(inMemoryPool, NULL),
        mValueWithRefCount(new ValueWithRefCount(NULL))
    {
    }

    SharedOwnershipStrategy(MemoryPool & inMemoryPool, Value * inValue) :
        Base(inMemoryPool, inValue),
        mValueWithRefCount(new ValueWithRefCount(inValue))
    {
    }

    SharedOwnershipStrategy(const SharedOwnershipStrategy & rhs) :
        Base(rhs),
        mValueWithRefCount(rhs.mValueWithRefCount)
    {
        ++mValueWithRefCount->mRefCount;
    }

    SharedOwnershipStrategy& operator=(SharedOwnershipStrategy rhs)
    {
        Base::swap(rhs);
        This::swap(rhs);
        return *this;
    }

    ~SharedOwnershipStrategy()
    {
        if (--mValueWithRefCount->mRefCount)
        {
            Base::destroy();
        }
    }

    void swap(SharedOwnershipStrategy & rhs)
    {
        ValueWithRefCount * helper = mValueWithRefCount;
        mValueWithRefCount = rhs.mValueWithRefCount;
        rhs.mValueWithRefCount = helper;
    }

private:
    struct ValueWithRefCount : boost::noncopyable
    {
        ValueWithRefCount(Value * inValue) : mValue(inValue), mRefCount(1) { }

        Value * mValue;
        std::size_t mRefCount;
    };

    ValueWithRefCount * mValueWithRefCount;
};


template<class MemoryPoolType>
struct ScopedOwnershipStrategy : public BasicOwnershipStrategy<MemoryPoolType>,
                                 private boost::noncopyable
{
    typedef ScopedOwnershipStrategy<MemoryPoolType> This;
    typedef BasicOwnershipStrategy<MemoryPoolType> Base;

    typedef MemoryPoolType MemoryPool;
    typedef typename MemoryPool::Value Value;

    ScopedOwnershipStrategy(MemoryPool & inMemoryPool, Value * inValue) :
        Base(inMemoryPool, inValue)
    {
    }

    ~ScopedOwnershipStrategy()
    {
        Base::destroy();
    }
};


/**
 * MemoryPool that priorizes fast destruction.
 * Actual memory allocated is 2x the usable size.
 *
 */
template<typename ValueType>
class MemoryPool : boost::noncopyable
{
public:
    typedef ValueType Value;
    typedef MemoryPool<Value> This;
    typedef MemoryPool_SmartPointer<MemoryPool<ValueType>, Value, ScopedOwnershipStrategy<MemoryPool<ValueType> > > ScopedPtr;
    typedef MemoryPool_SmartPointer<MemoryPool<ValueType>, Value, SharedOwnershipStrategy<MemoryPool<ValueType> > > SharedPtr;

    MemoryPool(std::size_t inItemCount) :
        mData(sizeof(Value) * inItemCount),
        mItems(),
        mUsedItems(),
        mFreeItems(inItemCount, NULL)
    {
        for (std::size_t idx = 0; idx < inItemCount; ++idx)
        {
            char * data = &mData[idx * sizeof(Value)];
            Value * value = reinterpret_cast<Value*>(data);
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
    Value * acquire()
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

    Value * acquireAndDefaultConstruct()
    {
        return new (acquire()) Value();
    }

    /**
     * Acquires a memory slab and constructs the object using the FactoryFunction object.
     * The FactoryFunction signature must be "Value* (void*);" or "Value* (Value*);"
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
    Value * acquireAndConstruct(FactoryFunction inFactoryFunction)
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
    void release(const Value * inValue)
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

    void destructAndRelease(const Value * inValue)
    {
        inValue->~Value();
        release(inValue);
    }

    std::size_t indexOf(const Value * inValue) const
    {
        return inValue - reinterpret_cast<const Value*>(mData.data());
    }

    std::size_t offsetOf(const Value * inValue) const
    {
        return indexOf(inValue) * sizeof(Value);
    }

private:
    // Contigious container containing all data.
    std::vector<char> mData;

    class Item
    {
    public:
        Item(Value * inValue, std::size_t inIndex) :
            mValue(inValue),
            mIndex(inIndex),
            mUsed(false)
        {
        }

        Value * mValue;
        std::size_t mIndex;
        bool mUsed;
    };

    std::vector<Item> mItems;

    typedef std::vector<std::size_t> UsedItems;
    UsedItems mUsedItems;

    typedef std::vector<std::size_t> FreeItems;
    FreeItems mFreeItems;
};


} // namespace Futile


#endif // MemoryPoolTypeH_INCLUDED
