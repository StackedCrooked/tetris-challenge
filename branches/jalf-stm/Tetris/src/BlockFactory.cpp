#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Poco/Timestamp.h"
#include "stm.hpp"
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <ctime>


#ifndef TETRIS_BLOCKFACTORY_RANDOMIZE
#define TETRIS_BLOCKFACTORY_RANDOMIZE 0 // Temporary disable randomization
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

    stm::shared<BlockTypes> mBag;
    stm::shared<unsigned> mCurrentIndex;
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
    BlockType result = stm::atomic<BlockType>([&](stm::transaction & tx) {
        unsigned & currentIndex = mImpl->mCurrentIndex.open_rw(tx);
        if (currentIndex < mImpl->mBagSize)
        {
            const BlockTypes & bag = mImpl->mBag.open_r(tx);
            return bag[currentIndex++];
        }
        else
        {
            // Reshuffle the bag.
            BlockTypes & bag = mImpl->mBag.open_rw(tx);
            #if TETRIS_BLOCKFACTORY_RANDOMIZE
            std::random_shuffle(bag.begin(), bag.end());
            #endif
            return bag[currentIndex = 0];
        }
    });
    Assert(result >= BlockType_Begin && result < BlockType_End);
    return result;
}


} // namespace Tetris
