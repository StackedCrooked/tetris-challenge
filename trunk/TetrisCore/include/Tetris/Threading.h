#ifndef TETRIS_THREADING_H_INCLUDED
#define TETRIS_THREADING_H_INCLUDED


#include "Tetris/Assert.h"
#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>
#include <memory>


namespace Tetris
{

    template<class Variable>
    struct WithMutex
    {
        WithMutex(std::auto_ptr<Variable> inVariable) :
            mVariable(inVariable.release())
        {
        }

        boost::scoped_ptr<Variable> mVariable;
        mutable boost::timed_mutex mMutex;
    };

    // Forward declaration.
    template<class Variable>
    class ScopedConstAtom;

    // Forward declaration.
    template<class Variable>
    class ScopedAtom;


    template<class Variable>
    class Protected
    {
    public:
        Protected(std::auto_ptr<Variable> inVariable) :
            mVariableWithMutex(new WithMutex<Variable>(inVariable))
        {
        }

        // Default constructor can only be used if Variable has a default constructor.
        Protected() :
            mVariableWithMutex(new WithMutex<Variable>(std::auto_ptr<Variable>(new Variable)))
        {
        }

        boost::timed_mutex & getMutex() const
        { return mVariableWithMutex->mMutex; }

    private:
        friend class ScopedAtom<Variable>;
        friend class ScopedConstAtom<Variable>;
        boost::shared_ptr<WithMutex<Variable> > mVariableWithMutex;
    };

    
    // Simple stopwatch class.
    // Helper for ScopedAtom and ScopedConstAtom.
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
    class ScopedAtom
    {
    public:
        ScopedAtom(Protected<Variable> & inProtectedVariable) :
            mLock(inProtectedVariable.mVariableWithMutex->mMutex),
            mVariable(inProtectedVariable.mVariableWithMutex->mVariable.get())
        {

        }

        ~ScopedAtom()
        {
            Assert(mStopwatch.elapsedTimeMs() < cMaximumLockDurationMs);
        }

        Variable * get()
        { return mVariable; }

        Variable * operator->()
        { return mVariable; }
    private:
        ScopedAtom(const ScopedAtom&);
        ScopedAtom& operator=(const ScopedAtom&);

        boost::timed_mutex::scoped_lock mLock;
        Variable * mVariable;
        Stopwatch mStopwatch;
    };


    template<class Variable>
    class ScopedConstAtom
    {
    public:
        ScopedConstAtom(const Protected<Variable> & inProtectedVariable) :
            mLock(inProtectedVariable.mVariableWithMutex->mMutex),
            mVariable(inProtectedVariable.mVariableWithMutex->mVariable.get())
        {
        }

        ~ScopedConstAtom()
        {
            Assert(mStopwatch.elapsedTimeMs() < cMaximumLockDurationMs);
        }

        const Variable * get() const
        { return mVariable; }

        const Variable * operator->() const
        { return mVariable; }
    private:
        ScopedConstAtom(const ScopedConstAtom&);
        ScopedConstAtom& operator=(const ScopedConstAtom&);

        boost::timed_mutex::scoped_lock mLock;
        const Variable * mVariable;
        Stopwatch mStopwatch;
    };

} // namespace Tetris


#endif // THREADING_H_INCLUDED
