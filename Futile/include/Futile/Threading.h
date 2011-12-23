#ifndef THREADING_H
#define THREADING_H


#include "Futile/Assert.h"
#include "Futile/Types.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <set>


namespace Futile {


/**
 * Sleep
 *
 * Suspends the current thread for the given duration in milliseconds.
 */
void Sleep(UInt64 inMilliseconds);


/**
 * GetCurrentTimeMs
 *
 * Returns the current time in milliseconds.
 */
UInt64 GetCurrentTimeMs();


class LifeTimeChecker : boost::noncopyable
{
public:
    LifeTimeChecker();

    ~LifeTimeChecker();

private:
    UInt64 mBeginTime;
    UInt64 mMaxDuration;
};


typedef boost::recursive_mutex Mutex;
typedef boost::recursive_mutex::scoped_lock ScopedLock;


/**
 * LockMany is a mutex lock that can lock multiple mutexes.
 */
template<class MutexType>
class LockMany : boost::noncopyable
{
public:
    typedef std::vector<MutexType *> Mutexes;
    typedef typename Mutexes::size_type size_type;

    LockMany();

    ~LockMany();

    void lock(MutexType & inMutex);

    void unlockAll();

    size_type size() const;

private:
    Mutexes mMutexes;
};


template<class MutexType>
LockMany<MutexType>::LockMany()
{
}


template<class MutexType>
LockMany<MutexType>::~LockMany()
{
    unlockAll();
}


template<class MutexType>
void LockMany<MutexType>::lock(MutexType & inMutex)
{
    inMutex.lock();
    mMutexes.push_back(&inMutex);
}


template<class MutexType>
void LockMany<MutexType>::unlockAll()
{
    while (!mMutexes.empty())
    {
        mMutexes.back()->unlock();
        mMutexes.pop_back();
    }
}


template<class MutexType>
typename LockMany<MutexType>::size_type LockMany<MutexType>::size() const
{
    return mMutexes.size();
}


// Forward declarations
class LockerBase;
template<class> class Locker;


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

    const Locker<Variable> lock() const;

    Locker<Variable> lock();

    bool operator== (const ThreadSafe<Variable> & rhs) const
    {
        return mImpl.get() == rhs.mImpl.get();
    }

    bool operator!= (const ThreadSafe<Variable> & rhs) const
    {
        return !(*this == rhs);
    }

    bool compare(const ThreadSafe<Variable> & inOther) const
    {
        return mImpl < inOther.mImpl;
    }

    void reset()
    {
        mImpl.reset();
    }

private:
    friend class Locker<Variable>;
    friend class LockerBase;

    Mutex & getMutex() const
    {
        return mImpl->mMutex;
    }

    Variable * getVariable() const
    {
        return mImpl->mVariable;
    }

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

        mutable Variable * mVariable;
        Mutex mMutex;
    };

    boost::shared_ptr<Impl> mImpl;
};


template<class Variable>
bool operator< (const ThreadSafe<Variable> & lhs, const ThreadSafe<Variable> & rhs)
{
    return lhs.compare(rhs);
}


// This base class is used for the ScopeGuard trick in the FUTILE_LOCK macro.
class LockerBase
{
};


/**
 * Locker is used get access to the object wrapped by the ThreadSafe class.
 */
template<class Variable>
class Locker : public LockerBase
{
public:
    Locker(ThreadSafe<Variable> inTSV) :
        mThreadSafe(inTSV)
    {
        mThreadSafe.getMutex().lock();
    }

    Locker(Locker && rhs) :
        mThreadSafe(rhs.mThreadSafe)
    {
    }

    ~Locker()
    {
        if (mThreadSafe.mImpl)
        {
            mThreadSafe.getMutex().unlock();
        }
    }

    const Variable * get() const
    {
        return mThreadSafe.getVariable();
    }

    Variable * get()
    {
        return mThreadSafe.getVariable();
    }

    const Variable * operator->() const
    {
        return get();
    }

    Variable * operator->()
    {
        return get();
    }

private:
    // non-copyable
    Locker(const Locker&);
    Locker & operator=(const Locker &);

    mutable ThreadSafe<Variable> mThreadSafe;
};


template<class Variable>
const Locker<Variable> ThreadSafe<Variable>::lock() const
{
    return Locker<Variable>(*this);
}


template<class Variable>
Locker<Variable> ThreadSafe<Variable>::lock()
{
    return Locker<Variable>(*this);
}


namespace Helper {


template<class T>
struct Identity
{
    typedef T Type;
};


template<class T>
Identity<T> EncodeType(const T &)
{
    return Identity<T>();
}


struct CamelionType
{
    template<class T>
    operator Identity<T>() const
    {
        return Identity<T>();
    }
};


template<class T>
Futile::Locker<T> Lock(const Futile::ThreadSafe<T> & inTSV)
{
    return Futile::Locker<T>(inTSV);
}


template<class T>
T & Unwrap(const LockerBase & inLockerBase, const Identity< ThreadSafe<T> > &)
{
    const T * result = static_cast< const Locker<T> & >(inLockerBase).get();
    return const_cast<T &>(*result);
}


} // namespace Helper


/**
 * Macro that returns the type of an expression
 * without evaluating the expression.
 */
#define FUTILE_ENCODEDTYPEOF(EXPRESSION) \
  (true ? Futile::Helper::CamelionType() : Futile::Helper::EncodeType(EXPRESSION))


/**
 * Macro for creating conflict-free scopes
 */
#define FUTILE_FOR_BLOCK(DECL) \
    if (bool _c_ = false) ; else for(DECL;!_c_;_c_=true)


/**
 * FUTILE_LOCK can be used to create an atomic scope for accessing a thread-safe object.
 *
 * Usage example:
 *
 *   ThreadSafe<Foo> theFoo(new Foo);
 *
 *   // Atomic scope
 *   FUTILE_LOCK(Foo & foo, theFoo)
 *   {
 *       foo.bar();
 *   }
 */
#define FUTILE_LOCK(DECL, TSV) \
    FUTILE_FOR_BLOCK(const Futile::LockerBase & theLockerBase = Futile::Helper::Lock(TSV)) \
        FUTILE_FOR_BLOCK(DECL = Futile::Helper::Unwrap(theLockerBase, FUTILE_ENCODEDTYPEOF(TSV)))


} // namespace Futile


#endif // THREADING_H
