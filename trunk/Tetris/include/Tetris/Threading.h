#ifndef TETRIS_THREADING_H_INCLUDED
#define TETRIS_THREADING_H_INCLUDED


#include "Tetris/Assert.h"
#include "Tetris/MainThread.h"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/detail/atomic_count.hpp>
#include <memory>
#include <set>


namespace Tetris {


template<class SubType>
class InstanceTracker
{
public:
    InstanceTracker()
    {
        sInstances.insert(static_cast<SubType*>(this));
    }

    virtual ~InstanceTracker()
    {
        sInstances.erase(this);
    }

    static bool Exists(SubType * inInstance)
    {
        return sInstances.find(inInstance) != sInstances.end();
    }

private:
    typedef std::set<SubType*> Instances;
    static Instances sInstances;
};


template<class SubType>
typename InstanceTracker<SubType>::Instances InstanceTracker<SubType>::sInstances;


template<class Variable>
struct WithMutex
{
    WithMutex(std::auto_ptr<Variable> inVariable) :
        mVariable(inVariable.release())
    {
    }

    WithMutex(Variable * inVariable) :
        mVariable(inVariable)
    {
    }

    boost::scoped_ptr<Variable> mVariable;
    mutable boost::shared_mutex mMutex;
};

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
        mVariableWithMutex(new WithMutex<Variable>(inVariable)),
        mIdentifier(++sInstanceCount)
    {
    }

    ThreadSafe(Variable * inVariable) :
        mVariableWithMutex(new WithMutex<Variable>(inVariable)),
        mIdentifier(++sInstanceCount)
    {
    }

    // Default constructor can only be used if Variable has a default constructor.
    ThreadSafe() :
        mVariableWithMutex(new WithMutex<Variable>(std::auto_ptr<Variable>(new Variable))),
        mIdentifier(++sInstanceCount)
    {
    }

    bool operator== (const ThreadSafe<Variable> & rhs) const
    { return id() == rhs.id(); }

    bool operator!= (const ThreadSafe<Variable> & rhs) const
    { return !(*this == rhs); }

    operator bool() const
    {
        return mVariableWithMutex.get() != 0;
    }

    size_t id() const
    { return mIdentifier; }

    boost::timed_mutex & getMutex() const
    { return mVariableWithMutex->mMutex; }

private:
    friend class ScopedReaderAndWriter<Variable>;
    friend class ScopedReader<Variable>;
    boost::shared_ptr<WithMutex<Variable> > mVariableWithMutex;
    size_t mIdentifier;
    static boost::detail::atomic_count sInstanceCount;
};


// Static member initialization.
template<class Variable>
boost::detail::atomic_count ThreadSafe<Variable>::sInstanceCount(0);


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
        mSharedLock(inProtectedVariable.mVariableWithMutex->mMutex),
        mUpgradeLock(mSharedLock),
        mVariable(inProtectedVariable.mVariableWithMutex->mVariable.get())
    {

    }

    ~ScopedReaderAndWriter()
    {
        //Assert(mStopwatch.elapsedTimeMs() < cMaximumLockDurationMs);
    }

    Variable & operator *()
    { return mVariable; }

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
    ScopedReader(const ThreadSafe<Variable> & inProtectedVariable) :
        mSharedLock(inProtectedVariable.mVariableWithMutex->mMutex),
        mVariable(inProtectedVariable.mVariableWithMutex->mVariable.get())
    {
    }

    ~ScopedReader()
    {
        //Assert(mStopwatch.elapsedTimeMs() < cMaximumLockDurationMs);
    }

    const Variable & operator *() const
    { return mVariable; }

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


/**
 * LockMany is similar to a scoped_lock. It can however lock multiple mutexes.
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
