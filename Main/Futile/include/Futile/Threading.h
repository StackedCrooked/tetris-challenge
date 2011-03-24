#ifndef FUTILE_THREADING_H_INCLUDED
#define FUTILE_THREADING_H_INCLUDED


#include "Futile/Assert.h"
#include "Futile/MainThread.h"
#include "Futile/Logging.h"
#include "Poco/AtomicCounter.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <set>


namespace Futile {


typedef boost::mutex Mutex;
typedef boost::mutex::scoped_lock ScopedLock;
typedef boost::shared_mutex RWMutex;
typedef boost::condition_variable Condition;


class ScopedRWLock : boost::noncopyable
{
public:
    ScopedRWLock(RWMutex & inRWMutex, bool inWrite = false) :
        mReadLock(inRWMutex)
    {
        if (inWrite)
        {
            mWriteLock.reset(new WriteLock(mReadLock));
        }
    }

    ~ScopedRWLock()
    {
    }

private:
    typedef boost::upgrade_lock<RWMutex> ReadLock;
    ReadLock mReadLock;

    typedef boost::upgrade_to_unique_lock<RWMutex> WriteLock;
    boost::scoped_ptr<WriteLock> mWriteLock;
};


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
    typedef ScopedReader<Variable> ScopedReader;
    typedef ScopedWriter<Variable> ScopedWriter;

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
    { return id() == rhs.id(); }

    bool operator!= (const ThreadSafe<Variable> & rhs) const
    { return !(*this == rhs); }

    operator bool() const
    {
        return mImpl.get() != 0;
    }

    size_t id() const
    { return mImpl->mIdentifier; }

private:
    // This is the base class for the ScopedReader and ScopedWriter classes.
    friend class ScopedAccessor<Variable>;

    struct Impl : boost::noncopyable
    {
        Impl(std::auto_ptr<Variable> inVariable) :
            mVariable(inVariable.release()),
            mIdentifier(++sInstanceCount)
        {
        }

        Impl(Variable * inVariable) :
            mVariable(inVariable),
            mIdentifier(++sInstanceCount)
        {
        }

        ~Impl()
        {
            delete mVariable;
        }

        Variable * mVariable;
        RWMutex mRWMutex;
        size_t mIdentifier;
    };

    boost::shared_ptr<Impl> mImpl;
    static Poco::AtomicCounter sInstanceCount;
};


// Static member initialization.
template<class Variable>
Poco::AtomicCounter ThreadSafe<Variable>::sInstanceCount(0);


template<class Variable>
bool operator< (const ThreadSafe<Variable> & lhs, const ThreadSafe<Variable> & rhs)
{
    return lhs.id() < rhs.id();
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
    RWMutex & getRWMutex()
    {
        return mProtectedVariable.mImpl->mRWMutex;
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


enum LockType
{
    LockType_ReadLock,
    LockType_WriteLock
};


/**
 * ConfigurableScopedAccessor sits between the ScopedReader/ScopedReadAndWriter and the ScopedAccessor classes.
 * The policy template arguments can be used to specify whether and how the class should do extra runtime checking
 * and error reporting.
 */
template<class Variable,
         LockType inLockType,
         class CheckLockDurationPolicy = TimeLimitMs<10>,
         class CheckLockOrderPolicy    = VoidPolicy>
class ConfigurableScopedAccessor : public  ScopedAccessor<Variable>,
                                   private CheckLockDurationPolicy,
                                   private CheckLockOrderPolicy
{
public:
    typedef ScopedAccessor<Variable> Super;
    ConfigurableScopedAccessor(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable),
        mScopedRWLock(ScopedAccessor<Variable>::getRWMutex(), inLockType == LockType_WriteLock)
    {
    }

protected:
    ScopedRWLock mScopedRWLock;
};


/**
 * ScopedReader creates a read-lock during the lifetime of its object.
 * Access to the wrapped object is restricted to const-ref.
 *
 * Be careful if your object has mutable data!
 */
template<class Variable>
class ScopedReader : public ConfigurableScopedAccessor<Variable, LockType_ReadLock>
{
private:
    typedef ConfigurableScopedAccessor<Variable, LockType_ReadLock> Super;

public:
    ScopedReader(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable)
    {
    }

    // Access is limited to const-ref.
    const Variable & operator *() const
    { return *ScopedAccessor<Variable>::getVariable(); }

    const Variable * get() const
    { return ScopedAccessor<Variable>::getVariable(); }

    const Variable * operator->() const
    { return ScopedAccessor<Variable>::getVariable(); }
};


/**
 * ScopedWriter creates, during it's lifetime, a unique-lock
 * on the shared resource and provides full access to the object.
 */
template<class Variable>
class ScopedWriter : public ConfigurableScopedAccessor<Variable, LockType_WriteLock>
{
private:
    typedef ConfigurableScopedAccessor<Variable, LockType_WriteLock> Super;

public:
    ScopedWriter(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable)
    {
    }

    Variable & operator *()
    { return *ScopedAccessor<Variable>::getVariable(); }

    Variable * get()
    { return ScopedAccessor<Variable>::getVariable(); }

    Variable * operator->()
    { return ScopedAccessor<Variable>::getVariable(); }
};


/**
 * InstanceTracker allows you to monitor instances of a class.
 * To use it the monitored class must inherit InstanceTracker.
 */
class InstanceTracker : boost::noncopyable
{
public:
    typedef std::set<InstanceTracker*> Instances;
    typedef ThreadSafe<Instances> ThreadedInstances;

    InstanceTracker()
    {
        ScopedWriter<Instances> rwInstances(sInstances);
        rwInstances->insert(this);
    }

    virtual ~InstanceTracker()
    {
        ScopedWriter<Instances> rwInstances(sInstances);
        rwInstances->erase(this);
    }

    static bool HasInstance(InstanceTracker * inInstance)
    {
        ScopedReader<Instances> rInstances(sInstances);
        return rInstances->find(inInstance) != rInstances->end();
    }

private:
    static ThreadedInstances sInstances;
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
