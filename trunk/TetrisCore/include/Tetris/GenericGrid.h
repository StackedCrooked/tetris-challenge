#ifndef GENERICGRID_H_INCLUDED
#define GENERICGRID_H_INCLUDED


#include <vector>


namespace Tetris
{

    /**
     * Represents a grid of values.
     * Indices are zero-based.
     */
    template<class T>
    class GenericGrid
    {
    public:
        typedef std::vector<T> Data;

        GenericGrid(size_t numRows, size_t numColumns);

        GenericGrid(size_t numRows, size_t numColumns, const T & inInitialValue);

        const T & get(size_t row, size_t col) const;

        T & get(size_t row, size_t col);

        void set(size_t row, size_t col, const T & inT);

        size_t numRows() const;

        size_t numColumns() const;

        const Data & data() const;

        Data & data();

    private:
        size_t mNumRows;
        size_t mNumColumns;
        Data mData;
    };


    template<class T>
    GenericGrid<T>::GenericGrid(size_t numRows, size_t numColumns):
        mNumRows(numRows),
        mNumColumns(numColumns)
    {
        mData.resize(numRows*numColumns);
    }


    template<class T>
    GenericGrid<T>::GenericGrid(size_t numRows, size_t numColumns, const T & inInitialValue):
        mNumRows(numRows),
        mNumColumns(numColumns)
    {
        mData.resize(numRows*numColumns, inInitialValue);
    }


    template<class T>
    const T & GenericGrid<T>::get(size_t rowIdx, size_t colIdx) const
    {
        return mData[rowIdx*mNumColumns + colIdx];
    }


    template<class T>
    T & GenericGrid<T>::get(size_t rowIdx, size_t colIdx)
    {
        return mData[rowIdx*mNumColumns + colIdx];
    }


    template<class T>
    void GenericGrid<T>::set(size_t rowIdx, size_t colIdx, const T & inT)
    {
        mData[rowIdx * mNumColumns + colIdx] = inT;
    }


    template<class T>
    size_t GenericGrid<T>::numRows() const
    {
        return mNumRows;
    }


    template<class T>
    size_t GenericGrid<T>::numColumns() const
    {
        return mNumColumns;
    }

    
    template<class T>
    const std::vector<T> & GenericGrid<T>::data() const
    {
        return mData;
    }

    
    template<class T>
    std::vector<T> & GenericGrid<T>::data()
    {
        return mData;
    }


} // namespace Tetris

#endif // GENERICGRID_H_INCLUDED
