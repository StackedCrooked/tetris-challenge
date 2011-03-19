#ifndef FUTILE_THREADING_H_INCLUDED
#define FUTILE_THREADING_H_INCLUDED


#include "Futile/ThreadingConfiguration.h"
#include "Futile/Assert.h"
#include "Futile/MainThread.h"
#include "Futile/Logging.h"
#include "Poco/AtomicCounter.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <set>


namespace Futile {


// Forward declarations
template<class Variable>
class ScopedAccessor;


/**
 * ThreadSafe can be used to enapsulate an object that needs to be accessed by multiple threads.
 * The object is stored as a private member variable and access can only be obtained by using
 * one of two friend classes: ScopedReader and ScopedReaderAndWriter.
 * ScopedReader is a shared-read-lock and gives you const access the the object.
 * ScopedReaderAndWriter is a unique lock that gives you full access to the object.
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
	// This is the base class for the ScopedReader and ScopedReaderAndWriter classes.
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
        SharedMutex mSharedMutex;
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
// Used by the TimeLimitPolicy class of the ScopedReaderAndWriter and ScopedReader classes.
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
template<
	class Variable,
	class CheckLockDurationPolicy = TimeLimitMs<10>,
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
    typedef ConfigurableScopedAccessor<Variable> Super;

    ScopedReader(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable),
        mSharedLock(ScopedAccessor<Variable>::getSharedMutex())
    {
    }

    ~ScopedReader()
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
    SharedLock mSharedLock;
};

	
/**
 * ScopedReaderAndWriter creates, during it's lifetime, a unique-lock
 * on the shared resource and provides full access to the object.
 */
template<class Variable>
class ScopedReaderAndWriter : public ScopedReader<Variable>
{
public:
    typedef ScopedReader<Variable> Super;

    ScopedReaderAndWriter(ThreadSafe<Variable> inProtectedVariable) :
        Super(inProtectedVariable),
        mUniqueLock(Super::mSharedLock)
    {
    }

    ~ScopedReaderAndWriter()
    {
    }

	// Full access to the wrapped object.
    Variable & operator *()
    { return *ScopedAccessor<Variable>::getVariable(); }

    Variable * get()
    { return ScopedAccessor<Variable>::getVariable(); }

    Variable * operator->()
    { return ScopedAccessor<Variable>::getVariable(); }

private:
    UniqueLock mUniqueLock;
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
        ScopedReaderAndWriter<Instances> rwInstances(sInstances);
        rwInstances->insert(this);
    }

    virtual ~InstanceTracker()
    {
        ScopedReaderAndWriter<Instances> rwInstances(sInstances);
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
    LockMutex(inMutex);
    mMutexes.push_back(&inMutex);
}


template<class Mutex>
void LockMany<Mutex>::unlockAll()
{
    while (!mMutexes.empty())
    {
        UnlockMutex(*mMutexes.back());
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
