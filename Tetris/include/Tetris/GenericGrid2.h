#ifndef TETRIS_GENERICGRID2_H_INCLUDED
#define TETRIS_GENERICGRID2_H_INCLUDED


#include <cassert>
#include <cstring>
#include <vector>
#include <memory>


namespace Tetris {


template<class T, template <class> class Allocator>
class Grid : private Allocator<T>
{
public:
    Grid(std::size_t inRowCount, std::size_t inColumnCount);

    Grid(std::size_t inRowCount, std::size_t inColumnCount, const T & inInitialValue);

    ~Grid();

    std::size_t rowCount() const;

    std::size_t columnCount() const;

    const T & get(std::size_t inRow, std::size_t inColumn);

    void set(std::size_t inRow, std::size_t inColumn, const T & inValue);

private:
    Grid(const Grid&);
    Grid& operator=(const Grid&);

    std::size_t mRowCount;
    std::size_t mColumnCount;
    T * mBuffer;
};


template<class T>
class Allocator_Malloc
{
public:
    Allocator_Malloc(size_t inSize) :
        mSize(sizeof(T) * inSize)
    {
    }

    T * alloc()
    {
        return reinterpret_cast<T*>(malloc(mSize));
    }

    void free(T * inBuffer)
    {
        free(inBuffer);
    }

private:
    size_t mSize;
};


template<class T>
class Allocator_New
{
public:
    Allocator_New(size_t inSize) :
        mSize(sizeof(T) * inSize)
    {
    }

    T * alloc()
    {
        return new T[mSize];
    }

    void free(T * inBuffer)
    {
        delete [] inBuffer;
    }

private:
    size_t mSize;
};


//
// Inlines
//
template<class T, template <class> class Allocator>
Grid<T, Allocator>::Grid(std::size_t inRowCount, std::size_t inColumnCount) :
    Allocator<T>(inRowCount * inColumnCount),
    mRowCount(inRowCount),
    mColumnCount(inColumnCount),
    mBuffer(Allocator<T>::alloc())
{
}


template<class T, template <class> class Allocator>
Grid<T, Allocator>::Grid(std::size_t inRowCount, std::size_t inColumnCount, const T & inInitialValue) :
    Allocator<T>(inRowCount * inColumnCount),
    mRowCount(inRowCount),
    mColumnCount(inColumnCount),
    mBuffer(Allocator<T>::alloc())
{
    // We can't use a memcpy on class objects, we must use copy constructor instead.
    std::size_t size = inRowCount * inColumnCount;
    for (size_t idx = 0; idx != size; ++idx)
    {
        mBuffer[idx] = inInitialValue;
    }
}


template<class T, template <class> class Allocator>
Grid<T, Allocator>::~Grid()
{
    Allocator<T>::free(mBuffer);
}


template<class T, template <class> class Allocator>
std::size_t Grid<T, Allocator>::rowCount() const
{
    return mRowCount;
}


template<class T, template <class> class Allocator>
std::size_t Grid<T, Allocator>::columnCount() const
{
    return mColumnCount;
}


template<class T, template <class> class Allocator>
const T & Grid<T, Allocator>::get(std::size_t inRow, std::size_t inColumn)
{
    assert(inRow < mRowCount && inColumn < mColumnCount);
    return mBuffer[inRow * mColumnCount + inColumn];
}


template<class T, template <class> class Allocator>
void Grid<T, Allocator>::set(std::size_t inRow, std::size_t inColumn, const T & inValue)
{
    assert(inRow < mRowCount && inColumn < mColumnCount);
    mBuffer[inRow * mColumnCount + inColumn] = inValue;
}


} // namespace Tetris


#endif // GENERICGRID_H_INCLUDED
