#ifndef TETRIS_GENERICGRID_H_INCLUDED
#define TETRIS_GENERICGRID_H_INCLUDED


#include "Tetris/Allocator.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <memory>


namespace Tetris {


template<class T, template <class> class Allocator = Allocator_Vector>
class GenericGrid : private Allocator<T>
{
public:
    GenericGrid(std::size_t inRowCount, std::size_t inColumnCount);

    GenericGrid(std::size_t inRowCount, std::size_t inColumnCount, const T & inInitialValue);

    ~GenericGrid();

    std::size_t rowCount() const;

    std::size_t columnCount() const;

    const T & get(std::size_t inRow, std::size_t inColumn) const;

    void set(std::size_t inRow, std::size_t inColumn, const T & inValue);

private:
    std::size_t mRowCount;
    std::size_t mColumnCount;
    T * mBuffer;
};


//
// Inlines
//
template<class T, template <class> class Allocator>
GenericGrid<T, Allocator>::GenericGrid(std::size_t inRowCount, std::size_t inColumnCount) :
    Allocator<T>(inRowCount * inColumnCount),
    mRowCount(inRowCount),
    mColumnCount(inColumnCount),
    mBuffer(Allocator<T>::alloc())
{
}


template<class T, template <class> class Allocator>
GenericGrid<T, Allocator>::GenericGrid(std::size_t inRowCount, std::size_t inColumnCount, const T & inInitialValue) :
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
GenericGrid<T, Allocator>::~GenericGrid()
{
    Allocator<T>::free(mBuffer);
}


template<class T, template <class> class Allocator>
std::size_t GenericGrid<T, Allocator>::rowCount() const
{
    return mRowCount;
}


template<class T, template <class> class Allocator>
std::size_t GenericGrid<T, Allocator>::columnCount() const
{
    return mColumnCount;
}


template<class T, template <class> class Allocator>
const T & GenericGrid<T, Allocator>::get(std::size_t inRow, std::size_t inColumn) const
{
    assert(inRow < mRowCount && inColumn < mColumnCount);
    return mBuffer[inRow * mColumnCount + inColumn];
}


template<class T, template <class> class Allocator>
void GenericGrid<T, Allocator>::set(std::size_t inRow, std::size_t inColumn, const T & inValue)
{
    assert(inRow < mRowCount && inColumn < mColumnCount);
    mBuffer[inRow * mColumnCount + inColumn] = inValue;
}


} // namespace Tetris


#endif // GENERICGRID_H_INCLUDED
