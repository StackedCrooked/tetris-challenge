#include "Tetris/Config.h"
#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include <algorithm>
#include <ctime>


namespace Tetris {


AbstractBlockFactory::AbstractBlockFactory()
{
}


AbstractBlockFactory::~AbstractBlockFactory()
{
}


class BlockFactoryImpl
{
public:
    BlockFactoryImpl(int inBagSize);

    BlockType getNext();

private:
    size_t mBagSize;
    size_t mCurrentIndex;
    BlockTypes mBag;
};


BlockFactoryImpl::BlockFactoryImpl(int inBagSize) :
    mBagSize(inBagSize),
    mCurrentIndex(0)
{
    // Pick a new seed.
    srand(static_cast<unsigned int>(time(NULL)));

    size_t totalSize = mBagSize * cBlockTypeCount;
    mBag.reserve(totalSize);
    for (size_t idx = 0; idx != totalSize; ++idx)
    {
        BlockType blockType = static_cast<BlockType>(1 + (idx % cBlockTypeCount));
        mBag.push_back(blockType);
    }
    std::random_shuffle(mBag.begin(), mBag.end());
}


BlockType BlockFactoryImpl::getNext()
{
    if (mCurrentIndex >= mBag.size())
    {
        // Reshuffle the bag.
        std::random_shuffle(mBag.begin(), mBag.end());
        mCurrentIndex = 0;
    }
    return mBag[mCurrentIndex++];
}


BlockFactory::BlockFactory(int inBagSize) :
    AbstractBlockFactory(),
    mImpl(new BlockFactoryImpl(inBagSize))
{
}


BlockFactory::~BlockFactory()
{
    delete mImpl;
    mImpl = 0;
}


BlockType BlockFactory::getNext()
{
    return mImpl->getNext();
}


} // namespace Tetris
