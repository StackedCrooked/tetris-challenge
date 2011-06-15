#ifndef THREADING_H_INCLUDED
#define THREADING_H_INCLUDED


#include "Futile/Assert.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <set>


namespace Futile {


typedef boost::mutex Mutex;
typedef boost::mutex::scoped_lock ScopedLock;
typedef boost::condition_variable Condition;


// Forward declarations
class LockerBase;
template<class> class Locker;


// Lock/Unlock function. These could be overloaded for different mutex types.
inline void Lock  (boost::mutex & ioMutex) { ioMutex.lock();   }
inline void Unlock(boost::mutex & ioMutex) { ioMutex.unlock(); }


/**
 * ThreadSafe can be used to add a thread-safe wrapper around an object.
 *
 * The protected object is stored as a private member variable along with a mutex.
 * A Locker object can be used to obtain access to the protected object.
 */
template<class Variable>
class ThreadSafe
{
public:
    // Constructor that takes an autoptr object.
    explicit ThreadSafe(std::auto_ptr<Variable> inVariable) :
        mImpl(new Impl(inVariable))
    {
    }

    // Constructor that takes the new object.
    explicit ThreadSafe(Variable * inVariable) :
        mImpl(new Impl(inVariable))
    {
    }

    // Default constructor can only be used if Variable has a default constructor.
    explicit ThreadSafe() :
        mImpl(new Impl(new Variable))
    {
    }

    Locker<Variable> lock();

    bool operator== (const ThreadSafe<Variable> & rhs) const
    { return mImpl.get() == rhs.mImpl.get(); }

    bool operator!= (const ThreadSafe<Variable> & rhs) const
    { return !(*this == rhs); }

    bool compare(const ThreadSafe<Variable> & inOther)
    { return mImpl < inOther.mImpl; }

private:
    friend class Locker<Variable>;
    friend class LockerBase;

    struct Impl : boost::noncopyable
    {
        Impl(std::auto_ptr<Variable> inVariable) :
            mVariable(inVariable.release())
        {
            Assert(mVariable);
        }

        Impl(Variable * inVariable) :
            mVariable(inVariable)
        {
            Assert(mVariable);
        }

        ~Impl()
        {
            delete mVariable;
        }

        Variable * mVariable;
        Mutex mSharedMutex;
    };

    boost::shared_ptr<Impl> mImpl;
};


template<class Variable>
bool operator< (const ThreadSafe<Variable> & lhs, const ThreadSafe<Variable> & rhs)
{
    return lhs.compare(rhs);
}


// Simple stopwatch class.
// Used by the TimeLimitPolicy class of the Locker and Locker classes.
class Stopwatch : boost::noncopyable
{
public:
    Stopwatch();

    ~Stopwatch();

    unsigned elapsedTimeMs() const;

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


/**
 * VoidPolicy can be used as a default type for policies that you don't want to set.
 */
 template<int n = 0>
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


// Helper for the LOCK macro
class LockerBase
{
};


/**
 * Locker is the base class for the Locker and ScopedReadedAndWriter classes.
 */
template<class Variable>
class Locker : public LockerBase
{
public:
    Locker(ThreadSafe<Variable> inProtectedVariable) :
        mProtectedVariable(inProtectedVariable)
    {
    }

    // Allow copy to enable move semantics
    Locker(const Locker & rhs) :
        mProtectedVariable(rhs.mProtectedVariable)
    {
    }

    const Variable * get() const { return mProtectedVariable.mImpl->mVariable; }

    Variable * get() { return mProtectedVariable.mImpl->mVariable; }

    const Variable * operator->() const { return get(); }

    Variable * operator->() { return get(); }

private:
    // disallow assignment
    Locker& operator=(const Locker&);
    ThreadSafe<Variable> mProtectedVariable;
};


template<class Variable>
Locker<Variable> ThreadSafe<Variable>::lock()
{
    return Locker<Variable>(*this);
}


template<class Variable>
Locker<Variable> TSLock(ThreadSafe<Variable> inTSV)
{
    return Locker<Variable>(inTSV);
}


namespace Helper {


using namespace Futile;


template<class Variable>
static Locker<Variable> Create(ThreadSafe<Variable> & inVariable)
{
    return Locker<Variable>(inVariable);
}

template<class Variable>
static Variable & GetVariable(const LockerBase & base, ThreadSafe<Variable> & tsv)
{
    const Variable & var = *static_cast< const Locker<Variable> & >(base).get();
    return const_cast<Variable&>(var);
}


} // namespace Helper


/**
 * Helper
 */
#define FUTILE_FOR_BLOCK(DECL) \
    if(bool _c_ = false) ; else for(DECL;!_c_;_c_=true)


/**
 * LOCK helps with the creation of locks.
 */
#define FUTILE_LOCK(DECL, TSV) \
    FUTILE_FOR_BLOCK(const Futile::Helper::LockerBase & base = Futile::Helper::Create(TSV)) \
        FUTILE_FOR_BLOCK(DECL = Futile::Helper::GetVariable(base, TSV))


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
    Lock(inMutex);
    mMutexes.push_back(&inMutex);
}


template<class Mutex>
void LockMany<Mutex>::unlockAll()
{
    while (!mMutexes.empty())
    {
        Unlock(*mMutexes.back());
        mMutexes.pop_back();
    }
}


template<class Mutex>
typename LockMany<Mutex>::size_type LockMany<Mutex>::size() const
{
    return mMutexes.size();
}


} // namespace Futile


#endif // THREADING_H_INCLUDED
