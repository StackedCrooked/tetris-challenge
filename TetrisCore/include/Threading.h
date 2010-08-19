#ifndef THREADING_H_INCLUDED
#define THREADING_H_INCLUDED


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
        mutable boost::mutex mMutex;
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

    private:
        friend class ScopedAtom<Variable>;
        friend class ScopedConstAtom<Variable>;
        boost::shared_ptr<WithMutex<Variable> > mVariableWithMutex;
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

        Variable * get()
        { return mVariable; }

        Variable * operator->()
        { return mVariable; }
    private:
        boost::mutex::scoped_lock mLock;
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

        const Variable * get() const
        { return mVariable; }

        const Variable * operator->() const
        { return mVariable; }
    private:
        boost::mutex::scoped_lock mLock;
        const Variable * mVariable;
    };

} // namespace Tetris


#endif // THREADING_H_INCLUDED
