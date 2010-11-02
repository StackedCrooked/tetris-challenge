#ifndef TETRIS_GENERICGRID_H_INCLUDED
#define TETRIS_GENERICGRID_H_INCLUDED


#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>
#include <map>


namespace Tetris
{
    
    class MemoryPoolImpl;

    class MemoryPool
    {
    public:
        MemoryPool(size_t inItemSize);

        ~MemoryPool();

        void* get();

        void release(void* inData);

    private:
        MemoryPool(const MemoryPool&);
        MemoryPool& operator=(const MemoryPool&);

        MemoryPoolImpl * mImpl;
    };


    template<class T>
    class GenericGrid
    {
    public:
        GenericGrid(size_t inNumRows, size_t inNumColumns);

        GenericGrid(size_t inNumRows, size_t inNumColumns, const T & inInitialValue);

        ~GenericGrid();

        GenericGrid(const GenericGrid&);
        
        GenericGrid& operator=(const GenericGrid&);

        inline const T & get(size_t inRow, size_t inColumn) const
        {
            const T * t = reinterpret_cast<const T*>(mData);
            return t[inRow * mNumColumns + inColumn];
        }

        inline void set(size_t inRow, size_t inColumn, const T & inValue)
        {
            T * t = reinterpret_cast<T*>(mData);
            t[inRow * mNumColumns + inColumn] = inValue;
        }

        inline size_t numColumns() const
        {
            return mNumColumns;
        }

        inline size_t numRows() const
        {
            return mNumRows;
        }

    private:
        size_t mNumRows;
        size_t mNumColumns;
        size_t mItemSize;
        boost::scoped_ptr<MemoryPool> mMemoryPool;
        void * mData;
    };


    template<class T>
    GenericGrid<T>::GenericGrid(size_t inNumRows, size_t inNumColumns) :
        mNumRows(inNumRows),
        mNumColumns(inNumColumns),
        mItemSize(sizeof(T) * inNumColumns * inNumRows),
        mMemoryPool(new MemoryPool(mItemSize)),
        mData(mMemoryPool->get())
    {
    }


    template<class T>
    GenericGrid<T>::GenericGrid(size_t inNumRows, size_t inNumColumns, const T & inInitialValue) :
        mNumRows(inNumRows),
        mNumColumns(inNumColumns),
        mItemSize(sizeof(T) * inNumColumns * inNumRows),
        mMemoryPool(new MemoryPool(mItemSize)),
        mData(mMemoryPool->get())
    {        
        for (size_t i = 0; i < (mNumColumns * mNumRows); ++i)
        {
            T * data = reinterpret_cast<T*>(mData);
            data[i] = inInitialValue;
        }
    }


    template<class T>
    GenericGrid<T>::~GenericGrid()
    {
        mMemoryPool->release(mData);
        mMemoryPool.reset();
    }
    
    
    template<class T>
    GenericGrid<T>::GenericGrid(const GenericGrid& rhs) :
        mNumRows(rhs.mNumRows),
        mNumColumns(rhs.mNumColumns),
        mItemSize(rhs.mItemSize),
        mMemoryPool(new MemoryPool(mItemSize)),
        mData(mMemoryPool->get())
    {
        memcpy(mData, rhs.mData, mItemSize);
    }


    template<class T>
    GenericGrid<T>& GenericGrid<T>::operator=(const GenericGrid& rhs)
    {
        if (this != &rhs)
        {
            mNumRows = rhs.mNumRows;
            mNumColumns = rhs.mNumColumns;
            mItemSize = rhs.mItemSize;
            mMemoryPool.reset(new MemoryPool(mItemSize));
            mData = mMemoryPool->get();
            memcpy(mData, rhs.mData, mItemSize);
        }
        return *this;
    }

} // namespace Tetris


#endif // GENERICGRID_H_INCLUDED
