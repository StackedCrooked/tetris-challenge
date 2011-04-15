#ifndef FUTILE_ARRAY_H
#define FUTILE_ARRAY_H


#include <vector>


namespace Futile {


/**
 * Array<T> is a wrapper around std::vector<T*> and takes ownership of the stored objects.
 * The public interface is mostly similar to std::vector.
 */
template<class T>
class Array
{
public:

    Array() { }

    ~Array()
    {
        while (!mData.empty())
        {
            delete mData.back();
            mData.pop_back();
        }
    }
	
    typedef std::vector<T*> Data;
    typedef typename Data::iterator iterator;
    typedef typename Data::const_iterator const_iterator;

    iterator begin()
    {
        return mData.begin();
    }

    iterator end()
    {
        return mData.end();
    }

    const_iterator begin() const
    {
        return mData.begin();
    }

    const_iterator end() const
    {
        return mData.end();
    }

    T * operator [] (typename Data::size_type inIndex)
    {
        return mData[inIndex];
    }

    const T * operator[](typename Data::size_type inIndex) const
    {
        return mData[inIndex];
    }

    void push_back(T * inValue)
    {
        mData.push_back(inValue);
    }

    typename Data::size_type size() const
    {
        return mData.size();
    }

    void resize(typename Data::size_type inNewSize)
    {
        while (mData.size() > inNewSize)
        {
            delete mData.back();
            mData.pop_back();
        }

        // We still need to resize in case the new
        // size is bigger than the current size.
        mData.resize(inNewSize);
    }

    const T * back() const
    {
        return mData.back();
    }

    T * back()
    {
        return mData.back();
    }

    void erase(iterator it)
    {
        delete *it;
        mData.erase(it);
    }
	
private:
    Data mData;
};


} // namespace Futile


#endif // FUTILE_ARRAY_H

