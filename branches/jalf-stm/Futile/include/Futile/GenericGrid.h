#ifndef GENERICGRID_H
#define GENERICGRID_H


#include <cstring>
#include <vector>
#include <memory>


namespace Futile {


/**
 * GenericGrid represents a grid or table structure.
 */
template<class T>
class GenericGrid
{
public:
    GenericGrid(std::size_t inRowCount, std::size_t inColumnCount);

    GenericGrid(std::size_t inRowCount, std::size_t inColumnCount, const T & inInitialValue);

    inline std::size_t rowCount() const;

    inline std::size_t columnCount() const;

    inline const T & get(std::size_t inRow, std::size_t inColumn) const;

    inline void set(std::size_t inRow, std::size_t inColumn, const T & inValue);

private:
    typedef std::vector<T> Data;
    Data mData;
    std::size_t mRowCount;
    std::size_t mColumnCount;
};


//
// Inlines
//
template<class T>
GenericGrid<T>::GenericGrid(std::size_t inRowCount, std::size_t inColumnCount) :
    mData(inRowCount * inColumnCount),
    mRowCount(inRowCount),
    mColumnCount(inColumnCount)
{
}


template<class T>
GenericGrid<T>::GenericGrid(std::size_t inRowCount, std::size_t inColumnCount, const T & inInitialValue) :
    mData(inRowCount * inColumnCount, inInitialValue),
    mRowCount(inRowCount),
    mColumnCount(inColumnCount)
{
}


template<class T>
std::size_t GenericGrid<T>::rowCount() const
{
    return mRowCount;
}


template<class T>
std::size_t GenericGrid<T>::columnCount() const
{
    return mColumnCount;
}


template<class T>
const T & GenericGrid<T>::get(std::size_t inRow, std::size_t inColumn) const
{
    return mData[inRow * mColumnCount + inColumn];
}


template<class T>
void GenericGrid<T>::set(std::size_t inRow, std::size_t inColumn, const T & inValue)
{
    mData[inRow * mColumnCount + inColumn] = inValue;
}


} // namespace Futile


#endif // GENERICGRID_H
