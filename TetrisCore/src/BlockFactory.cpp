#include "Tetris/Config.h"
#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include <algorithm>
#include <ctime>


namespace Tetris
{
    
    class BlockFactoryImpl
    {
    public:
        BlockFactoryImpl(int inBagSize) :
            mBagSize(inBagSize),
            mCurrentIndex(0)
        {
        }

        size_t mBagSize;
        mutable size_t mCurrentIndex;
        mutable BlockTypes mBag;
    };


    BlockFactory::BlockFactory(int inBagSize) :
        mImpl(new BlockFactoryImpl(inBagSize))
    {
        // Pick a new seed.
        srand(static_cast<unsigned int>(time(NULL)));

        size_t totalSize = mImpl->mBagSize * cBlockTypeCount;
        mImpl->mBag.reserve(totalSize);
        for (size_t idx = 0; idx != totalSize; ++idx)
        {
            BlockType blockType = static_cast<BlockType>(1 + (idx % cBlockTypeCount));
            mImpl->mBag.push_back(blockType);
        }
        std::random_shuffle(mImpl->mBag.begin(), mImpl->mBag.end());
    }


    BlockType BlockFactory::getNext() const
    {
        if (mImpl->mCurrentIndex >= mImpl->mBag.size())
        {
            // Reshuffle the bag.
            std::random_shuffle(mImpl->mBag.begin(), mImpl->mBag.end());
            mImpl->mCurrentIndex = 0;
        }
        return mImpl->mBag[mImpl->mCurrentIndex++];
    }

} // namespace Tetris
