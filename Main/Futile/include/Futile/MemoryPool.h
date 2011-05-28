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
namespace MemoryPool {


template<class MemoryPoolType>
struct ScopedPtr;

template<class MemoryPoolType>
struct SharedPtr;

template<class MemoryPoolType>
struct MovePtr;


/**
 * WrappedPointer encapsulates a pointer value and provides various ways to access it.
 */
template<class Value>
struct WrappedPointer
{
    WrappedPointer(Value * inValue) :
        mValue(inValue)
    {
    }

    void swap(WrappedPointer & rhs)
    {
        std::swap(mValue, rhs.mValue);
    }

    inline const Value * get() const { return mValue; }

    inline Value * get() { return mValue; }

    inline const Value * operator->() const { return get(); }

    inline Value * operator->() { return get(); }

    inline const Value & operator*() const { return *get(); }

    inline Value & operator*() { return *get(); }

protected:
    void setValue(Value * inValue)
    {
        mValue = inValue;
    }

private:
    Value * mValue;
};


/**
 *
 */
template<class MemoryPoolType>
struct SmartPointer : public WrappedPointer<typename MemoryPoolType::Value>
{
public:
    typedef MemoryPoolType MemoryPool;
    typedef typename MemoryPoolType::Value Value;

private:
    typedef WrappedPointer<Value> Base;
    typedef SmartPointer<MemoryPool> This;

public:
    typedef ScopedPtr<MemoryPool> ScopedPtr;
    typedef SharedPtr<MemoryPool> SharedPtr;
    typedef MovePtr<MemoryPool> MovePtr;

    SmartPointer(MemoryPool & inMemoryPool) :
        Base(NULL),
        mMemoryPool(&inMemoryPool)
    {
    }

    SmartPointer(MemoryPool & inMemoryPool, Value * inValue) :
        Base(inValue),
        mMemoryPool(&inMemoryPool)
    {
    }

    void swap(SmartPointer & rhs)
    {
        Base::swap(rhs);
        std::swap(mMemoryPool, rhs.mMemoryPool);
    }

    void reset(Value * inValue)
    {
        destroy();
        setValue(inValue);
    }

private:
    inline void destroy()
    {
        Value * value = Base::get();
        if (value)
        {
            value->~Value();
            mMemoryPool->release(value);
        }
    }

    MemoryPool * mMemoryPool;
};


template<class MemoryPoolType>
struct MovePtr : public SmartPointer<MemoryPoolType>
{
private:
    typedef MovePtr<MemoryPoolType> This;
    typedef SmartPointer<MemoryPoolType> Base;

public:
    typedef MemoryPoolType MemoryPool;
    typedef typename MemoryPool::Value Value;

    MovePtr(MemoryPool & inMemoryPool, Value * inValue) :
        Base(inMemoryPool, inValue),
        mOwns(true)
    {
    }

    MovePtr(const MovePtr & rhs) :
        Base(rhs),
        mOwns(true)
    {
        rhs.mOwns = false;
    }

    ~MovePtr()
    {
        if (mOwns)
        {
            Base::destroy();
        }
    }

    Value * release()
    {
        mOwns = false;
        return Base::get();
    }

private:
    // Disable assignment and swap
    MovePtr & operator=(const MovePtr & rhs);
    void swap(This & rhs);

    mutable bool mOwns;
};


/**
 * SharedPtr for SharedPtr class.
 */
template<class MemoryPoolType>
struct SharedPtr : public SmartPointer<MemoryPoolType>
{
    typedef SmartPointer<MemoryPoolType> Base;
    typedef SharedPtr<MemoryPoolType> This;

    typedef MemoryPoolType MemoryPool;
    typedef typename MemoryPool::Value Value;

    typedef typename Base::MovePtr MovePtr;

    SharedPtr(MemoryPool & inMemoryPool) :
        Base(inMemoryPool, NULL),
        mValueWithRefCount(new ValueWithRefCount(NULL))
    {
    }

    SharedPtr(MemoryPool & inMemoryPool, Value * inValue) :
        Base(inMemoryPool, inValue),
        mValueWithRefCount(new ValueWithRefCount(inValue))
    {
    }

    SharedPtr(const SharedPtr & rhs) :
        Base(rhs),
        mValueWithRefCount(rhs.mValueWithRefCount)
    {
        ++mValueWithRefCount->mRefCount;
    }

    /**
     * Constructor taking a MovePtr
     */
    SharedPtr(MovePtr rhs) :
        Base(*rhs.mMemoryPool, rhs.get()),
        mValueWithRefCount(new ValueWithRefCount(rhs.release()))
    {
    }


    /**
     * Assignment operator
     */
    SharedPtr & operator=(SharedPtr rhs)
    {
        This::swap(rhs);
        return *this;
    }

    void swap(SharedPtr & rhs)
    {
        Base::swap(rhs);
        std::swap(mValueWithRefCount, rhs.mValueWithRefCount);
    }

    ~SharedPtr()
    {
        if (--mValueWithRefCount->mRefCount)
        {
            Base::destroy();
        }
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
struct ScopedPtr : public SmartPointer<MemoryPoolType>,
                   private boost::noncopyable
{
    typedef SmartPointer<MemoryPoolType> Base;
    typedef ScopedPtr<MemoryPoolType> This;

    typedef MemoryPoolType MemoryPool;
    typedef typename MemoryPool::Value Value;

    ScopedPtr(MemoryPool & inMemoryPool, Value * inValue) :
        Base(inMemoryPool, inValue)
    {
    }

    ScopedPtr(const typename Base::MovePtr & rhs) :
        Base(rhs)
    {
        rhs.mOwns = false;
    }

    ~ScopedPtr()
    {
        Base::destroy();
    }

private:
    // Disable swap and assignment operator
    void swap(ScopedPtr & rhs);
    ScopedPtr & operator=(ScopedPtr rhs);
};


/**
 * MemoryPool that priorizes fast destruction.
 */
template<typename ValueType>
class MemoryPool : boost::noncopyable
{
public:
    typedef ValueType Value;
    typedef MemoryPool<Value> This;
    typedef MovePtr<This> MovePtr;
    typedef ScopedPtr<This> ScopedPtr;
    typedef SharedPtr<This> SharedPtr;

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

    std::size_t indexOf(const Value * inValue) const
    {
        return inValue - reinterpret_cast<const Value*>(mData.data());
    }

    std::size_t indexOf(const WrappedPointer<Value> & inWrappedPointer) const
    {
        return indexOf(inWrappedPointer.get());
    }

    std::size_t offsetOf(const Value * inValue) const
    {
        return indexOf(inValue) * sizeof(Value);
    }

    std::size_t offsetOf(const WrappedPointer<Value> & inWrappedPointer) const
    {
        return offsetOf(inWrappedPointer.get());
    }

    /**
     * Gets a pointer to a unconstructed item on the pool.
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

    /**
     * Release the pointer. The object is assuemd to be destructed already!
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
            }
        }
    }

private:
    // Contigious container containing all data.
    std::vector<char> mData;

    class Item
    {
    public:
        Item(Value * inValue, std::size_t inIndex) :
            mValue(inValue),
            mIndex(inIndex)
        {
        }

        Value * mValue;
        std::size_t mIndex;
    };

    std::vector<Item> mItems;

    typedef std::vector<std::size_t> UsedItems;
    UsedItems mUsedItems;

    typedef std::vector<std::size_t> FreeItems;
    FreeItems mFreeItems;
};


/**
 * Acquire and call the default constructor
 */
template<class MemoryPoolType>
typename MemoryPoolType::MovePtr AcquireAndDefaultConstruct(MemoryPoolType & pool)
{
    typedef typename MemoryPoolType::Value Value;
    typedef typename MemoryPoolType::MovePtr MovePtr;
    return MovePtr(pool, new (pool.acquire()) Value());
}

/**
 * Acquires and construct with factory function.
 * The factory function signature must be <Value* (void*)> or <Value* (Value*)>
 *
 * Example:
 *
 *   // Factory function
 *   static Foo * Create(void * placement, const std::string & arg0, const std::string & arg1)
 *   {
 *     return new (placement) Foo(arg0, arg1)
 *   }
 *
 *   Point * point = pool.acquireAndConstruct(boost::bind(CreatePoint, "Hello", "World!"));
 *
 */
template<class MemoryPoolType, typename FactoryFunction>
typename MemoryPoolType::MovePtr AcquireAndConstructWithFactory(MemoryPoolType & pool, FactoryFunction function)
{
    typedef typename MemoryPoolType::MovePtr MovePtr;
    return MovePtr(pool, function(pool.acquire()));
}


/**
 * Destructs hte object and releases the pointer from the pool.
 */
template<class MemoryPoolType>
void DestructAndRelease(MemoryPoolType & pool, const typename MemoryPoolType::Value * value)
{
    typedef typename MemoryPoolType::Value Value;
    value->~Value();
    pool.release(value);
}


} } // namespace Futile::MemoryPool


#endif // MemoryPoolTypeH_INCLUDED
