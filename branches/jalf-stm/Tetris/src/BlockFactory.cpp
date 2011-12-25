#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Poco/Timestamp.h"
#include "stm.hpp"
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <ctime>


namespace Tetris {


using namespace Futile;


struct BlockFactory::Impl : boost::noncopyable
{
    typedef unsigned seed_t;

    static BlockTypes GetInitialBag(unsigned n)
    {
        BlockTypes result;
        for (int i = 0; i < n; ++i)
        {
            for (BlockType type = BlockType_Begin; type != BlockType_End; ++type)
            {
                result.push_back(type);
            }
        }
        std::random_shuffle(result.begin(), result.end());
        return result;
    }

    Impl(unsigned n) :
        mCurrentIndex(0), // trigger a reshuffle on first access
        mBag(GetInitialBag(n)),
        mBagSize(n * cBlockTypeCount),
        mSeed(static_cast<seed_t>(Poco::Timestamp().epochMicroseconds()))
    {
    }

    ~Impl()
    {
    }

    stm::shared<unsigned> mCurrentIndex;
    stm::shared<BlockTypes> mBag;
    const unsigned mBagSize;
    const seed_t mSeed;
};


BlockFactory::BlockFactory(unsigned inBagSize) :
    mImpl(new Impl(inBagSize))
{
    srand(mImpl->mSeed);
}


BlockFactory::~BlockFactory()
{
}


BlockType BlockFactory::getNext()
{
    return stm::atomic<BlockType>([&](stm::transaction & tx) {
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
            std::random_shuffle(bag.begin(), bag.end());
            currentIndex = 0;
            return bag[currentIndex];
        }
    });
}


} // namespace Tetris
