#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Poco/Timestamp.h"
#include "stm.hpp"
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <ctime>


#ifndef TETRIS_BLOCKFACTORY_RANDOMIZE
#define TETRIS_BLOCKFACTORY_RANDOMIZE 1 // Temporary disable randomization
#endif


namespace Tetris {


using namespace Futile;


struct BlockFactory::Impl : boost::noncopyable
{
    typedef unsigned seed_t;

    static BlockTypes GetInitialBag(unsigned n)
    {
        BlockTypes result;
        for (unsigned i = 0; i < n; ++i)
        {
            for (BlockType type = BlockType_Begin; type != BlockType_End; ++type)
            {
                result.push_back(type);
            }
        }
        Assert(result.size() == n * cBlockTypeCount);
        #if TETRIS_BLOCKFACTORY_RANDOMIZE
        std::random_shuffle(result.begin(), result.end());
        #endif
        return result;
    }

    Impl(unsigned n) :
        mBag(GetInitialBag(n)),
        mCurrentIndex(0),
        mBagSize(n * cBlockTypeCount),
        mSeed(static_cast<seed_t>(Poco::Timestamp().epochMicroseconds()))
    {
    }

    ~Impl()
    {
    }

    BlockTypes mBag;
    unsigned mCurrentIndex;
    const unsigned mBagSize;
    const seed_t mSeed;
};


BlockFactory::BlockFactory(unsigned inBagSize) :
    mImpl(new Impl(inBagSize))
{
    srand(mImpl->mSeed);
}


BlockType BlockFactory::getNext()
{
    if (mImpl->mCurrentIndex < mImpl->mBagSize)
    {
        return mImpl->mBag[mImpl->mCurrentIndex++];
    }
    else
    {
        // Reshuffle the bag.
        #if TETRIS_BLOCKFACTORY_RANDOMIZE
        std::random_shuffle(mImpl->mBag.begin(), mImpl->mBag.end());
        #endif
        mImpl->mCurrentIndex = 0;
        return mImpl->mBag[mImpl->mCurrentIndex++];
    }
}


} // namespace Tetris
