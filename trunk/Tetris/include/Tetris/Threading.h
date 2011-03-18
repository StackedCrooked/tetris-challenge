#ifndef TETRIS_THREADING_H_INCLUDED
#define TETRIS_THREADING_H_INCLUDED


#include "Tetris/Assert.h"
#include "Tetris/MainThread.h"
#include "Tetris/Logging.h"
#include "Tetris/Utilities.h"
#include "Poco/AtomicCounter.h"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <set>


namespace Tetris {


//
// Configuration options
//
typedef boost::shared_mutex SharedMutex;


// Forward declaration.
template<class Variable>
class ScopedReader;


// Forward declaration.
template<class Variable>
class ScopedReaderAndWriter;


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
    friend class ScopedReaderAndWriter<Variable>;
    friend class ScopedReader<Variable>;

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
        SharedMutex mMutex;
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
class StopwatchImpl;
class Stopwatch
{
public:
    Stopwatch();

    ~Stopwatch();

    int elapsedTimeMs() const;

private:
    Stopwatch(const Stopwatch&);
    Stopwatch& operator=(const Stopwatch&);

    StopwatchImpl * mImpl;
};


extern const int cMaximumLockDurationMs;


template<class Variable>
class ScopedReaderAndWriter
{
public:
    ScopedReaderAndWriter(ThreadSafe<Variable> inProtectedVariable) :
        mSharedLock(inProtectedVariable.mImpl->mMutex),
        mUpgradeLock(mSharedLock),
        mVariable(inProtectedVariable.mImpl->mVariable)
    {

    }

    ~ScopedReaderAndWriter()
    {
        Assert(mStopwatch.elapsedTimeMs() < cMaximumLockDurationMs);
    }

    Variable & operator *()
    { return *mVariable; }

    Variable * get()
    { return mVariable; }

    Variable * operator->()
    { return mVariable; }

private:
    ScopedReaderAndWriter(const ScopedReaderAndWriter&);
    ScopedReaderAndWriter& operator=(const ScopedReaderAndWriter&);

    boost::upgrade_lock<boost::shared_mutex> mSharedLock;
    boost::upgrade_to_unique_lock<boost::shared_mutex> mUpgradeLock;

    Variable * mVariable;
    Stopwatch mStopwatch;
};


template<class Variable>
class ScopedReader
{
public:
    ScopedReader(ThreadSafe<Variable> inProtectedVariable) :
        mSharedLock(inProtectedVariable.mImpl->mMutex),
        mVariable(inProtectedVariable.mImpl->mVariable)
    {
    }

    ~ScopedReader()
    {
        Assert(mStopwatch.elapsedTimeMs() < cMaximumLockDurationMs);
    }

    const Variable & operator *() const
    { return *mVariable; }

    const Variable * get() const
    { return mVariable; }

    const Variable * operator->() const
    { return mVariable; }

private:
    ScopedReader(const ScopedReader&);
    ScopedReader& operator=(const ScopedReader&);

    boost::upgrade_lock<boost::shared_mutex> mSharedLock;
    const Variable * mVariable;
    Stopwatch mStopwatch;
};


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
 * LockMany is a scoped mutex locker that can lock an arbitrary number of mutexes.
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


inline void LockMutex(boost::mutex & inMutex)
{
    inMutex.lock();
}


inline void UnlockMutex(boost::mutex & inMutex)
{
    inMutex.unlock();
}


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


} // namespace Tetris


#endif // THREADING_H_INCLUDED
