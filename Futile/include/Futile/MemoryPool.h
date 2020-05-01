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


template<typename ValueType> struct ScopedPtr;
template<typename ValueType> struct SharedPtr;
template<typename ValueType> struct MovePtr;
template<typename ValueType> class MemoryPool;


/**
 * WrappedPointer encapsulates a pointer value and provides various ways to access it.
 */
template<class Value>
struct WrappedPointer
{
    WrappedPointer(Value* inValue) :
        mValue(inValue)
    {
    }

    void swap(WrappedPointer& rhs)
    {
        std::swap(mValue, rhs.mValue);
    }

    inline const Value * get() const { return mValue; }

    inline Value * get() { return mValue; }

    inline const Value * operator->() const { return get(); }

    inline Value * operator->() { return get(); }

    inline const Value& operator*() const { return *get(); }

    inline Value& operator*() { return *get(); }

protected:
    void setValue(Value* inValue)
    {
        mValue = inValue;
    }

private:
    Value* mValue;
};


/**
 *
 */
template<class ValueType>
struct SmartPointer : public WrappedPointer<ValueType>
{
public:
    typedef ValueType Value;

private:
    typedef WrappedPointer<Value> Base;
    typedef SmartPointer<Value> This;

public:

    SmartPointer(MemoryPool<Value>& inMemoryPool) :
        Base(nullptr),
        mMemoryPool(&inMemoryPool)
    {
    }

    SmartPointer(MemoryPool<Value>& inMemoryPool, Value* inValue) :
        Base(inValue),
        mMemoryPool(&inMemoryPool)
    {
    }

    void swap(SmartPointer& rhs)
    {
        Base::swap(rhs);
        std::swap(mMemoryPool, rhs.mMemoryPool);
    }

    void reset(Value* inValue)
    {
        destroy();
        setValue(inValue);
    }

protected:
    inline void destroy()
    {
        Value * value = Base::get();
        if (value)
        {
            value->~Value();
            mMemoryPool->release(value);
        }
    }

    template<typename ValueType_>
    friend SharedPtr<ValueType_> Share(MovePtr<ValueType_> inMovePtr);

    MemoryPool<Value> * mMemoryPool;
};


template<class ValueType>
struct MovePtr : public SmartPointer<ValueType>
{
private:
    typedef MovePtr<ValueType> This;
    typedef SmartPointer<ValueType> Base;

public:
    typedef ValueType Value;

    MovePtr(MemoryPool<Value>& inMemoryPool, Value* inValue) :
        Base(inMemoryPool, inValue),
        mOwns(true)
    {
    }

    MovePtr(const MovePtr& rhs) :
        Base(rhs),
        mOwns(true)
    {
        rhs.mOwns = false;
    }

    ~MovePtr()
    {
        if (mOwns)
        {
            SmartPointer<Value>::destroy();
        }
    }

    Value * release()
    {
        mOwns = false;
        return Base::get();
    }

private:
    friend class ScopedPtr<Value>;

    // Disable assignment and swap
    MovePtr& operator=(const MovePtr& rhs);
    void swap(This& rhs);

    mutable bool mOwns;
};


/**
 * MovePtr factory function
 */
template<typename ValueType>
MovePtr<ValueType> Move(MemoryPool<ValueType>& pool, ValueType* inValue)
{
    return MovePtr<ValueType>(pool, inValue);
}


/**
 * SharedPtr for SharedPtr class.
 */
template<class ValueType>
struct SharedPtr : public SmartPointer<ValueType>
{
private:
    typedef SmartPointer<ValueType> Base;
    typedef SharedPtr<ValueType> This;

public:
    typedef ValueType Value;

    SharedPtr(MemoryPool<Value>& inMemoryPool) :
        Base(inMemoryPool, nullptr),
        mValueWithRefCount(new ValueWithRefCount(nullptr))
    {
    }

    SharedPtr(MemoryPool<Value>& inMemoryPool, Value* inValue) :
        Base(inMemoryPool, inValue),
        mValueWithRefCount(new ValueWithRefCount(inValue))
    {
    }

    SharedPtr(const SharedPtr& rhs) :
        Base(rhs),
        mValueWithRefCount(rhs.mValueWithRefCount)
    {
        ++mValueWithRefCount->mRefCount;
    }


    /**
     * Assignment operator
     */
    SharedPtr& operator=(SharedPtr rhs)
    {
        This::swap(rhs);
        return *this;
    }

    void swap(SharedPtr& rhs)
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
        ValueWithRefCount(Value* inValue) : mValue(inValue), mRefCount(1) { }

        Value* mValue;
        std::size_t mRefCount;
    };

    ValueWithRefCount* mValueWithRefCount;
};


/**
* "Share" is a factory function for creating SharedPtr objects.
 */
template<typename ValueType>
SharedPtr<ValueType> Share(MemoryPool<ValueType>& pool, ValueType* inValue)
{
    return SharedPtr<ValueType>(pool, inValue);
}


/**
 * "Share" overload that creates a SharedPtr from a MovePtr.
 */
template<typename ValueType>
SharedPtr<ValueType> Share(MovePtr<ValueType> inMovePtr)
{
    return Share(*inMovePtr.mMemoryPool, inMovePtr.release());
}


template<class ValueType>
struct ScopedPtr : public SmartPointer<ValueType>,
                   private boost::noncopyable
{
private:
    typedef SmartPointer<ValueType> Base;
    typedef ScopedPtr<ValueType> This;

public:
    typedef ValueType Value;

    ScopedPtr(MemoryPool<Value>& inMemoryPool, Value* inValue) :
        Base(inMemoryPool, inValue)
    {
    }

    ScopedPtr(const MovePtr<Value>& rhs) :
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
    void swap(ScopedPtr& rhs);
    ScopedPtr& operator=(ScopedPtr rhs);
};


/**
 * MemoryPool that priorizes fast destruction.
 */
template<typename ValueType>
class MemoryPool : boost::noncopyable
{
private:
    typedef MemoryPool<ValueType> This;

public:
    typedef ValueType Value;

    MemoryPool(std::size_t inItemCount) :
        mData(sizeof(Value)* inItemCount),
        mItems(),
        mUsedItems(),
        mFreeItems(inItemCount)
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

    std::size_t indexOf(const Value* inValue) const
    {
        return inValue - reinterpret_cast<const Value*>(mData.data());
    }

    std::size_t indexOf(const WrappedPointer<Value>& inWrappedPointer) const
    {
        return indexOf(inWrappedPointer.get());
    }

    std::size_t offsetOf(const Value* inValue) const
    {
        return indexOf(inValue) * sizeof(Value);
    }

    std::size_t offsetOf(const WrappedPointer<Value>& inWrappedPointer) const
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
    void release(const Value* inValue)
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
        Item(Value* inValue, std::size_t inIndex) :
            mValue(inValue),
            mIndex(inIndex)
        {
        }

        Value* mValue;
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
template<class ValueType>
MovePtr<ValueType> AcquireAndDefaultConstruct(MemoryPool<ValueType>& pool)
{
    return Move(pool, new (pool.acquire()) ValueType());
}

/**
 * Acquires and construct with factory function.
 * The factory function signature must be <Value* (void*)> or <Value* (Value*)>
 *
 * Example:
 *
 *   // Factory function
 *   static Foo* Create(void * placement, const std::string& arg0, const std::string& arg1)
 *   {
 *     return new (placement) Foo(arg0, arg1)
 *   }
 *
 *   Point * point = pool.acquireAndConstruct(boost::bind(CreatePoint, "Hello", "World!"));
 *
 */
template<class ValueType, typename FactoryFunction>
MovePtr<ValueType> AcquireAndConstructWithFactory(MemoryPool<ValueType>& pool, FactoryFunction function)
{
    return Move(pool, function(pool.acquire()));
}


/**
 * Destructs hte object and releases the pointer from the pool.
 */
template<class ValueType>
void DestructAndRelease(MemoryPool<ValueType>& pool, const ValueType * value)
{
    value->~ValueType();
    pool.release(value);
}


} } } // namespace Futile::Memory::Pool


#endif // MEMORYPOOL_H_INCLUDED
