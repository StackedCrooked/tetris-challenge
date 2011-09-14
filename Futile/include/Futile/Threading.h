#ifndef THREADING_H_INCLUDED
#define THREADING_H_INCLUDED


#include "Futile/Assert.h"
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
void Sleep(boost::uint64_t inMilliseconds);


/**
 * GetCurrentTimeMs
 *
 * Returns the current time in milliseconds.
 */
boost::uint64_t GetCurrentTimeMs();


typedef boost::mutex Mutex;
typedef boost::mutex::scoped_lock ScopedLock;
typedef boost::condition_variable Condition;


/**
 * LockMany is a mutex lock that can lock multiple mutexes.
 */
template<class Mutex>
class LockMany : boost::noncopyable
{
public:
    typedef std::vector<Mutex *> Mutexes;
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
typename LockMany<Mutex>::size_type LockMany<Mutex>::size() const
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

    bool compare(const ThreadSafe<Variable> & inOther)
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

    // Allow copy to enable move semantics
    Locker(const Locker & rhs) :
        mThreadSafe(rhs.mThreadSafe)
    {
        rhs.invalidate();
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
    // disallow assignment
    Locker & operator=(const Locker &);

    void invalidate() const
    {
        mThreadSafe.mImpl.reset();
    }

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
 *   ThreadSafe<Foo> theFoo = ...;
 *
 *   // Get access to Foo object in
 *   // an atomic thread-safe scope:
 *   FUTILE_LOCK(Foo & foo, theFoo)
 *   {
 *       foo.bar();
 *   }
 */
#define FUTILE_LOCK(DECL, TSV) \
    FUTILE_FOR_BLOCK(const Futile::LockerBase & theLockerBase = Futile::Helper::Lock(TSV)) \
        FUTILE_FOR_BLOCK(DECL = Futile::Helper::Unwrap(theLockerBase, FUTILE_ENCODEDTYPEOF(TSV)))


} // namespace Futile


#endif // THREADING_H_INCLUDED
