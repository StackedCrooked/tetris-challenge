#include "Tetris/BlockFactory.h"
#include "Tetris/Block.h"
#include <algorithm>
#include <ctime>


namespace Tetris
{

    BlockFactory::BlockFactory(int inBagSize) :
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


    BlockType BlockFactory::getNext() const
    {
        if (mCurrentIndex >= mBag.size())
        {
            // Reshuffle the bag.
            std::random_shuffle(mBag.begin(), mBag.end());
            mCurrentIndex = 0;
        }
        return mBag[mCurrentIndex++];
    }


} // namespace Tetris
