#ifndef FUTILE_ARRAY_H
#define FUTILE_ARRAY_H


#include <vector>


namespace Futile {


/**
 * Array is a wrapper for std::vector that always takes ownership of the entries.
 */
template<class T>
class Array
{
public:
    typedef std::vector<T*> Data;
    Data mData;

    Array() { }

    ~Array()
    {
        while (!mData.empty())
        {
            delete mData.back();
            mData.pop_back();
        }
    }

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

    T * operator [ ] (typename Data::size_type inIndex)
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
        mData.erase(it);
    }
};


} // namespace Futile


#endif // FUTILE_ARRAY_H
