#ifndef GENERICGRID_H_INCLUDED
#define GENERICGRID_H_INCLUDED


#include "Futile/Allocator.h"
#include <cstring>
#include <vector>
#include <memory>


namespace Futile {


/**
 * GenericGrid can be used for simple matrices or 2D arrays.
 * It provides nothing more than simple get/set methods.
 * The Allocator template argument allows you to choose a different memory allocation implementation.]
 * Check the Allocators.h header for other available options.
 */
template<class T, template <class> class Allocator = Allocator_Malloc>
class GenericGrid : private Allocator<T>
{
public:
    GenericGrid(std::size_t inRowCount, std::size_t inColumnCount);

    GenericGrid(std::size_t inRowCount, std::size_t inColumnCount, const T& inInitialValue);

    std::size_t rowCount() const;

    std::size_t columnCount() const;

    const T& get(std::size_t inRow, std::size_t inColumn) const;

    void set(std::size_t inRow, std::size_t inColumn, const T& inValue);

private:
    std::size_t mRowCount;
    std::size_t mColumnCount;
};


//
// Inlines
//
template<class T, template <class> class Allocator>
GenericGrid<T, Allocator>::GenericGrid(std::size_t inRowCount, std::size_t inColumnCount) :
    Allocator<T>(inRowCount* inColumnCount),
    mRowCount(inRowCount),
    mColumnCount(inColumnCount)
{
}


template<class T, template <class> class Allocator>
GenericGrid<T, Allocator>::GenericGrid(std::size_t inRowCount, std::size_t inColumnCount, const T& inInitialValue) :
    Allocator<T>(inRowCount* inColumnCount, inInitialValue),
    mRowCount(inRowCount),
    mColumnCount(inColumnCount)
{
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
const T& GenericGrid<T, Allocator>::get(std::size_t inRow, std::size_t inColumn) const
{
    return Allocator<T>::get(inRow * mColumnCount + inColumn);
}


template<class T, template <class> class Allocator>
void GenericGrid<T, Allocator>::set(std::size_t inRow, std::size_t inColumn, const T& inValue)
{
    Allocator<T>::set(inRow * mColumnCount + inColumn, inValue);
}


} // namespace Futile


#endif // GENERICGRID_H_INCLUDED
