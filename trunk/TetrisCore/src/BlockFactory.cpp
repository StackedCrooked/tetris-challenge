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
        mBag.reserve(mBagSize);
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
        std::random_shuffle(mBag.begin(), mBag.end());
        mCurrentIndex = 0;
    }

    
    Block BlockFactory::getNext() const
    {
        if (mCurrentIndex >= mBagSize)
        {
            const_cast<BlockFactory*>(this)->reset();
        }
        mPrevious.push_back(mBag[mCurrentIndex]);
        return Block(mBag[mCurrentIndex++], 0, 0, 0);
    }


    size_t BlockFactory::numCreated() const
    {
        return mPrevious.size();
    }

    
    Block BlockFactory::getPrevious(size_t inIndex) const
    {
        CheckArgument(inIndex < mPrevious.size(), "Invalid index for getPrevious.");
        return Block(mPrevious[inIndex], 0, 0, 0);
    }



} // namespace Tetris
