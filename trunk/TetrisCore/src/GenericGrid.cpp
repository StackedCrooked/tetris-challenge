#include "Tetris/Config.h"
#include "Tetris/GenericGrid.h"
#include "Poco/MemoryPool.h"


namespace Tetris
{

    class MemoryPoolImpl
    {
    public:
        MemoryPoolImpl(size_t inItemSize);
        
        inline Poco::MemoryPool * pool()
        { return mPool; }

    private:
        MemoryPoolImpl(const MemoryPoolImpl&);
        MemoryPoolImpl& operator=(const MemoryPoolImpl&);

        size_t mItemSize;
        Poco::MemoryPool * mPool;
    };
    

    typedef std::map<size_t, boost::shared_ptr<Poco::MemoryPool> > MemoryPools;
    static MemoryPools sTetris_Grid_MemoryPools;


    MemoryPoolImpl::MemoryPoolImpl(size_t inItemSize) :
        mItemSize(inItemSize)
    {
        if (sTetris_Grid_MemoryPools.find(mItemSize) == sTetris_Grid_MemoryPools.end())
        {
            // TODO: determine amount of free memory and use 60% or so...
            const size_t cPoolSize = 1 * 1000 * 1000;
            boost::shared_ptr<Poco::MemoryPool> memoryPool(new Poco::MemoryPool(mItemSize, cPoolSize, cPoolSize));
            sTetris_Grid_MemoryPools.insert(std::make_pair(mItemSize, memoryPool));
        }
        mPool = sTetris_Grid_MemoryPools[mItemSize].get();
    }


    MemoryPool::MemoryPool(size_t inSizeInBytes) :
        mImpl(new MemoryPoolImpl(inSizeInBytes))
    {
    }


    MemoryPool::~MemoryPool()
    {
        delete mImpl;
        mImpl = 0;
    }


    void* MemoryPool::get()
    {
        return mImpl->pool()->get();
    }


    void MemoryPool::release(void* inData)
    {
        mImpl->pool()->release(inData);
    }

} // namespace Tetris
