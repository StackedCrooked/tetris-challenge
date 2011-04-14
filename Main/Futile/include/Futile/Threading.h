#ifndef FUTILE_THREADING_H_INCLUDED
#define FUTILE_THREADING_H_INCLUDED


#include "Futile/Assert.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <set>


namespace Futile {


typedef boost::mutex Mutex;
typedef boost::mutex::scoped_lock ScopedLock;
typedef boost::shared_mutex SharedMutex;
typedef boost::condition_variable Condition;


// Forward declarations
template<class> class ScopedAccessor;
template<class> class ScopedReader;
template<class> class ScopedWriter;


/**
 * ThreadSafe can be used to enapsulate an object that needs to be accessed by multiple threads.
 * The object is stored as a private member variable and access can only be obtained by using
 * one of two friend classes: ScopedReader and ScopedWriter.
 * ScopedReader is a shared-read-lock and gives you const access the the object.
 * ScopedWriter is a unique lock that gives you full access to the object.
 */
template<class Variable>
class ThreadSafe
{
public:
    // Constructor that takes an autoptr object.
    ThreadSafe(std::auto_ptr<Variable> inVariable) :
        mImpl(new Impl(inVariable))
    {
    }

    // Constructor that takes the new object.
    ThreadSafe(Variable * inVariable) :
        mImpl(new Impl(inVariable))
    {
    }

    // Default constructor can only be used if Variable has a default constructor.
    ThreadSafe() :
        mImpl(new Impl(new Variable))
    {
    }

    bool operator== (const ThreadSafe<Variable> & rhs) const
    { return mImpl.get() == rhs.mImpl.get(); }

    bool operator!= (const ThreadSafe<Variable> & rhs) const
    { return !(*this == rhs); }

    bool compare(const ThreadSafe<Variable> & inOther)
    { return mImpl.get() < inOther.mImpl.get(); }

private:
    // This is the base class for the ScopedReader and ScopedWriter classes.
    friend class ScopedAccessor<Variable>;

    struct Impl : boost::noncopyable
    {
        Impl(std::auto_ptr<Variable> inVariable) :
            mVariable(inVariable.release())
        {
        }

        Impl(Variable * inVariable) :
            mVariable(inVariable)
        {
        }

        ~Impl()
        {
            delete mVariable;
        }

        Variable * mVariable;
        SharedMutex mSharedMutex;
    };

    boost::shared_ptr<Impl> mImpl;
};


template<class Variable>
bool operator< (const ThreadSafe<Variable> & lhs, const ThreadSafe<Variable> & rhs)
{
    return lhs.compare(rhs);
}


// Simple stopwatch class.
// Used by the TimeLimitPolicy class of the ScopedWriter and ScopedReader classes.
class Stopwatch : boost::noncopyable
{
public:
    Stopwatch();

    ~Stopwatch();

    int elapsedTimeMs() const;

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


/**
 * VoidPolicy can be used as a default type for policies that you don't want to set.
 */
class VoidPolicy
{
};


/**
 * TimeLimitMs checks the duration of its own object lifetime.
 * During destruction it is checked if the maximum allowed duration
 * (which is specified as a value-type template argument) was exceeded.
 * If yes, then an assert/exception will result.
 */
template<time_t MaxDurationMs>
struct TimeLimitMs
{
    ~TimeLimitMs()
    {
        Assert(mStopwatch.elapsedTimeMs() < MaxDurationMs);
    }

    // Stopwatch starts during construction.
    Stopwatch mStopwatch;
};


/**
 * ScopedAccessor is the base class for the ScopedReader and ScopedReadedAndWriter classes.
 */
template<class Variable>
class ScopedAccessor : boost::noncopyable
{
public:
    ScopedAccessor(ThreadSafe<Variable> inProtectedVariable) :
        mProtectedVariable(inProtectedVariable)
    {
    }

protected:
    SharedMutex & getSharedMutex()
    {
        return mProtectedVariable.mImpl->mSharedMutex;
    }

    const Variable * getVariable() const
    {
        return mProtectedVariable.mImpl->mVariable;
    }

    Variable * getVariable()
    {
        return mProtectedVariable.mImpl->mVariable;
    }

private:
    ThreadSafe<Variable> mProtectedVariable;
};


/**
 * ConfigurableScopedAccessor sits between the ScopedReader/ScopedReadAndWriter and the ScopedAccessor classes.
 * The policy template arguments can be used to specify whether and how the class should do extra runtime checking
 * and error reporting.
 */
template<class Variable,
         class CheckLockDurationPolicy = TimeLimitMs<200>,
         class CheckLockOrderPolicy    = VoidPolicy>
class ConfigurableScopedAccessor : public  ScopedAccessor<Variable>,
                                   private CheckLockDurationPolicy,
                                   private CheckLockOrderPolicy
{
public:
    typedef ScopedAccessor<Variable> Super;
    ConfigurableScopedAccessor(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable)
    {
    }
};


/**
 * ScopedReader creates a read-lock during the lifetime of its object.
 * Access to the wrapped object is restricted to const-ref.
 *
 * Be careful if your object has mutable data!
 */
template<class Variable>
class ScopedReader : public ConfigurableScopedAccessor<Variable>
{
public:
    ScopedReader(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable),
        mReadLock(ScopedAccessor<Variable>::getSharedMutex())
    {
    }

    // Access is limited to const-ref.
    const Variable & operator *() const
    { return *ScopedAccessor<Variable>::getVariable(); }

    const Variable * get() const
    { return ScopedAccessor<Variable>::getVariable(); }

    const Variable * operator->() const
    { return ScopedAccessor<Variable>::getVariable(); }

protected:
    typedef ConfigurableScopedAccessor<Variable> Super;
    boost::upgrade_lock<SharedMutex> mReadLock;

};


/**
 * ScopedWriter creates, during it's lifetime, a unique-lock on
 * the C++ object and provides both read and write access to it.
 */
template<class Variable>
class ScopedWriter : public ScopedReader<Variable>
{
public:
    ScopedWriter(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable),
        mWriteLock(ScopedReader<Variable>::mReadLock)
    {
    }

    Variable & operator *()
    { return *ScopedAccessor<Variable>::getVariable(); }

    Variable * get()
    { return ScopedAccessor<Variable>::getVariable(); }

    Variable * operator->()
    { return ScopedAccessor<Variable>::getVariable(); }


private:
    typedef ScopedReader<Variable> Super;
    boost::upgrade_to_unique_lock<SharedMutex> mWriteLock;
};


/**
 * AtomicPrimitive enables atomic get/set operations on variables that have a primitive datatype.
 */
template<class T>
class AtomicPrimitive : boost::noncopyable
{
public:
    AtomicPrimitive(T inValue = T()) :
        mValue(inValue)
    {
    }

    void set(T inValue)
    {
        ScopedLock lock(mMutex);
        mValue = inValue;
    }

    T get() const
    {
        ScopedLock lock(mMutex);
        return mValue;
    }

private:
    mutable Mutex mMutex;
    T mValue;
};


/**
 * LockMany is a mutex lock that can lock multiple mutexes.
 */
template<class Mutex>
class LockMany : boost::noncopyable
{
public:
    typedef std::vector<Mutex*> Mutexes;
    typedef typename Mutexes::size_type size_type;

    LockMany();

    ~LockMany();

    void lock(Mutex & inMutex);

    void unlockAll();

    const Mutex & get(size_type inIndex) const;

    Mutex & get(size_type inIndex);

    size_type size() const;

private:
    Mutexes mMutexes;
};


template<class Mutex>
LockMany<Mutex>::LockMany()
{
}


template<class Mutex>
LockMany<Mutex>::~LockMany()
{
    unlockAll();
}


template<class Mutex>
void LockMany<Mutex>::lock(Mutex & inMutex)
{
    inMutex.lock();
    mMutexes.push_back(&inMutex);
}


template<class Mutex>
void LockMany<Mutex>::unlockAll()
{
    while (!mMutexes.empty())
    {
        mMutexes.back()->unlock();
        mMutexes.pop_back();
    }
}


template<class Mutex>
const Mutex & LockMany<Mutex>::get(size_type inIndex) const
{
    return mMutexes[inIndex];
}


template<class Mutex>
Mutex & LockMany<Mutex>::get(size_type inIndex)
{
    return mMutexes[inIndex];
}


template<class Mutex>
typename LockMany<Mutex>::size_type LockMany<Mutex>::size() const
{
    return mMutexes.size();
}


} // namespace Futile


#endif // FUTILE_THREADING_H_INCLUDED
