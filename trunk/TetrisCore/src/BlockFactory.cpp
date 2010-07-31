#include "BlockFactory.h"
#include "Block.h"
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
    }

    
    std::auto_ptr<Block> BlockFactory::getNext() const
    {
        if (mCurrentIndex >= mBagSize)
        {
            std::random_shuffle(mBag.begin(), mBag.end());
            mCurrentIndex = 0;
        }
        return std::auto_ptr<Block>(new Block(mBag[mCurrentIndex++], 0));
    }


} // namespace Tetris
