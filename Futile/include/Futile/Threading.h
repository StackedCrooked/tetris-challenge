#ifndef TETRIS_THREADING_H_INCLUDED
#define TETRIS_THREADING_H_INCLUDED


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


template<class Variable>
class ThreadSafe
{
public:
    ThreadSafe(std::auto_ptr<Variable> inVariable) :
        mImpl(new Impl(inVariable))
    {
    }

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
// Helper for ScopedReaderAndWriter and ScopedReader.
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
 * TimeLimitMs is an object that checks the duration of its own lifetime.
 * During destruction it is checked if the duration has been exceeded.
 * If yes, then an assert is triggered.
 */
template<unsigned int MaxDurationMs>
struct TimeLimitMs
{
    ~TimeLimitMs()
    {
        Assert(mStopwatch.elapsedTimeMs() < MaxDurationMs);
    }

    // Stopwatch starts during construction.
    Stopwatch mStopwatch;
};


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


template<class Variable,
         class CheckLockDurationPolicy = TimeLimitMs<10>,
         class CheckLockOrderPolicy = VoidPolicy>
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

    const Variable & operator *() const
    { return *ScopedAccessor<Variable>::getVariable(); }

    const Variable * get() const
    { return ScopedAccessor<Variable>::getVariable(); }

    const Variable * operator->() const
    { return ScopedAccessor<Variable>::getVariable(); }

protected:
    SharedLock mSharedLock;
};


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
 * To use it the class must inherit InstanceTracker.
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
 * LockMany is a mutex lock that can lock an arbitrary number of mutexes.
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


#endif // TETRIS_THREADING_H_INCLUDED
