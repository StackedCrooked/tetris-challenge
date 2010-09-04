#ifndef THREADING_H_INCLUDED
#define THREADING_H_INCLUDED


#include "Tetris/ErrorHandling.h"
#include <memory>
#include <boost/thread.hpp>


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


    

    class LockTimeout : public std::runtime_error
    {
    public:
        explicit LockTimeout(const std::string & inMessage) :
            std::runtime_error(inMessage)
        {
        }
    
        virtual ~LockTimeout() throw() { }    
    };


    template<class Variable>
    class ScopedAtom
    {
    public:
        ScopedAtom(Protected<Variable> & inProtectedVariable) :
            mLock(inProtectedVariable.mVariableWithMutex->mMutex),
            mVariable(inProtectedVariable.mVariableWithMutex->mVariable.get())
        {

        }

        ScopedAtom(Protected<Variable> & inProtectedVariable, int inTimeoutMs) :
            mLock(inProtectedVariable.mVariableWithMutex->mMutex, boost::defer_lock),
            mVariable(inProtectedVariable.mVariableWithMutex->mVariable.get())
        {
            if (!mLock.timed_lock(boost::posix_time::milliseconds(inTimeoutMs)))
            {
                throw LockTimeout("Lock timout occured during ScopedAtom constructor.");
            }
        }

        Variable * get()
        { return mVariable; }

        Variable * operator->()
        { return mVariable; }
    private:
        boost::timed_mutex::scoped_lock mLock;
        Variable * mVariable;
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

        ScopedConstAtom(Protected<Variable> & inProtectedVariable, int inTimeoutMs) :
            mLock(inProtectedVariable.mVariableWithMutex->mMutex, boost::defer_lock),
            mVariable(inProtectedVariable.mVariableWithMutex->mVariable.get())
        {
            if (!mLock.timed_lock(boost::posix_time::milliseconds(inTimeoutMs)))
            {
                throw std::runtime_error("Lock timeout.");
            }
        }

        const Variable * get() const
        { return mVariable; }

        const Variable * operator->() const
        { return mVariable; }
    private:
        boost::timed_mutex::scoped_lock mLock;
        const Variable * mVariable;
    };

} // namespace Tetris


#endif // THREADING_H_INCLUDED
