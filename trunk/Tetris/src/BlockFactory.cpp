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
using Futile::ScopedReader;
using Futile::ScopedReaderAndWriter;


namespace Tetris {


AbstractBlockFactory::AbstractBlockFactory()
{
}


AbstractBlockFactory::~AbstractBlockFactory()
{
}


class RandomSeed
{
public:
    typedef unsigned seed_t;
    static const seed_t cMaxSeed = 1000000; // I don't need that many

    RandomSeed() :
        mSeed(0)
    {
        Poco::Timestamp ts;
        Poco::Timestamp::TimeVal max(cMaxSeed);
        mSeed = static_cast<seed_t>(ts.epochMicroseconds() % max);
    }

    seed_t get()
    {
        mSeed = (mSeed + 1) % cMaxSeed;
        return mSeed;
    }

    static seed_t GetRandomSeed()
    {
        static Mutex fMutex;
        ScopedLock lock(fMutex);
        static RandomSeed fRandomSeed;
        return fRandomSeed.get();
    }

private:
    seed_t mSeed;
};


struct BlockFactory::Impl : boost::noncopyable
{
    Impl(int inBagSize) :
        mBagSize(inBagSize),
        mCurrentIndex(0)
    {
    }

    ~Impl()
    {
    }

    BlockTypes::size_type mBagSize;
    size_t mCurrentIndex;
    BlockTypes mBag;
};


BlockFactory::BlockFactory(int inBagSize) :
    AbstractBlockFactory(),
    mThreadSafeImpl(new Impl(inBagSize))
{
    ScopedReaderAndWriter<Impl> rwImpl(mThreadSafeImpl);
    Impl * mImpl(rwImpl.get());
    srand(RandomSeed::GetRandomSeed());

    size_t totalSize = mImpl->mBagSize * cBlockTypeCount;
    mImpl->mBag.reserve(totalSize);
    for (size_t idx = 0; idx != totalSize; ++idx)
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
    ScopedReaderAndWriter<Impl> rwImpl(mThreadSafeImpl);
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
