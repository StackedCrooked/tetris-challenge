#include "Tetris/Config.h"
#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Poco/Timestamp.h"
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <ctime>


using Futile::Mutex;
using Futile::ScopedLock;
using Futile::Locker;


namespace Tetris {


AbstractBlockFactory::AbstractBlockFactory()
{
}


AbstractBlockFactory::~AbstractBlockFactory()
{
}


struct BlockFactory::Impl : boost::noncopyable
{
    typedef unsigned seed_t;

    Impl(int inBagSize) :
        mBagSize(inBagSize),
        mCurrentIndex(0),
        mSeed(static_cast<seed_t>(Poco::Timestamp().epochMicroseconds()))
    {
    }

    ~Impl()
    {
    }

    BlockTypes::size_type mBagSize;
    std::size_t mCurrentIndex;
    BlockTypes mBag;
    seed_t mSeed;
};


BlockFactory::BlockFactory(int inBagSize) :
    AbstractBlockFactory(),
    mThreadSafeImpl(new Impl(inBagSize))
{
    Locker<Impl> rwImpl(mThreadSafeImpl);
    Impl * mImpl(rwImpl.get());
    srand(mImpl->mSeed);

    std::size_t totalSize = mImpl->mBagSize * cBlockTypeCount;
    mImpl->mBag.reserve(totalSize);
    for (std::size_t idx = 0; idx != totalSize; ++idx)
    {
        BlockType blockType = static_cast<BlockType>(1 + (idx % cBlockTypeCount));
        mImpl->mBag.push_back(blockType);
    }
    std::random_shuffle(mImpl->mBag.begin(), mImpl->mBag.end());
}


BlockFactory::~BlockFactory()
{
}


BlockType BlockFactory::getNext() const
{
    Locker<Impl> rwImpl(mThreadSafeImpl);
    Impl * mImpl(rwImpl.get());
    if (mImpl->mCurrentIndex >= mImpl->mBag.size())
    {
        // Reshuffle the bag.
        std::random_shuffle(mImpl->mBag.begin(), mImpl->mBag.end());
        mImpl->mCurrentIndex = 0;
    }
    return mImpl->mBag[mImpl->mCurrentIndex++];
}


} // namespace Tetris
