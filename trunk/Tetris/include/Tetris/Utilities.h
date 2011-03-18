#ifndef TETRIS_UTILITIES_H_INCLUDED
#define TETRIS_UTILITIES_H_INCLUDED


namespace Tetris {


template<class T>
static T DivideByTwo(T inValue)
{
    return static_cast<int>(0.5 + 0.5 * inValue);
}


class noncopyable
{
protected:
    noncopyable()
    {
    }

    ~noncopyable()
    {
    }

private:  // emphasize the following members are private
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};


template<class T>
class shared_ptr
{
public:
    struct Impl
    {
        Impl(T * inValue) :
            mRefCount(1),
            mValue(inValue)
        {
        }
        int mRefCount;
        T * mValue;
    };

    shared_ptr(T * inValue = 0) :
        mImpl(new Impl(inValue))
    {
    }

    shared_ptr(const shared_ptr & rhs) :
        mImpl(rhs.mImpl)
    {
        mImpl->mRefCount++;
    }

    ~shared_ptr()
    {
        unref();
    }

    shared_ptr & operator=(const shared_ptr & rhs)
    {
        if (this != &rhs)
        {
            rhs.mImpl->mRefCount++;
            unref();
            mImpl = rhs.mImpl;
        }
        return *this;
    }

    bool operator==(const shared_ptr & rhs)
    {
        return mImpl == rhs.mImpl;
    }

    operator bool() const
    {
        return mImpl != 0;
    }

    T * get()
    {
        return mImpl->mValue;
    }

    const T * get() const
    {
        return mImpl->mValue;
    }

    void reset()
    {
        unref();
    }

    void reset(T * inValue)
    {
        unref();
        mImpl = new Impl(inValue);
    }

    T & operator* () const
    {
        return *mImpl->mValue;
    }

    T * operator-> () const
    {
        return mImpl->mValue;
    }

private:
    inline void unref()
    {
        if (--mImpl->mRefCount == 0)
        {
            delete mImpl->mValue;
            delete mImpl;
            mImpl = 0;
        }
    }

    Impl * mImpl;
};


} // namespace Tetris


#endif // UTILITIES_H_INCLUDED
