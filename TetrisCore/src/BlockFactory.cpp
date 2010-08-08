#include "BlockFactory.h"
#include "Block.h"
#include "ErrorHandling.h"
#include <algorithm>


namespace Tetris
{

    BlockFactory::BlockFactory(int inBagSize) :        
        mBagSize(inBagSize),
        mCurrentIndex(0)
    {
        mBag.reserve(cBlockTypeCount * mBagSize);
        reset();
    }


    void BlockFactory::reset()
    {
        mBag.clear();
        for (size_t idx = 0; idx != mBagSize; ++idx)
        {
            BlockType blockType = static_cast<BlockType>(1 + (idx % cBlockTypeCount));
            mBag.push_back(blockType);
        }        
        mCurrentIndex = 0;
        std::random_shuffle(mBag.begin(), mBag.end());
    }

    
    BlockType BlockFactory::getNext() const
    {
        if (mCurrentIndex >= mBagSize)
        {
            // Reshuffle the bag.
            const_cast<BlockFactory*>(this)->reset();
        }
        return mBag[mCurrentIndex++];
    }


} // namespace Tetris
