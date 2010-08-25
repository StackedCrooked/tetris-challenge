#include "Tetris/BlockFactory.h"
#include "Tetris/Block.h"
#include "Tetris/ErrorHandling.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>


namespace Tetris
{

    BlockFactory::BlockFactory(int inBagSize) :        
        mBagSize(inBagSize),
        mCurrentIndex(0)
    {
        // Pick a new seed.
        srand(static_cast<unsigned int>(time(NULL)));

        mBag.reserve(cBlockTypeCount * mBagSize);
        reset();
    }


    void BlockFactory::reset()
    {
        mBag.clear();
        size_t totalSize = mBagSize * cBlockTypeCount;
        for (size_t idx = 0; idx != totalSize; ++idx)
        {
            BlockType blockType = static_cast<BlockType>(1 + (idx % cBlockTypeCount));
            mBag.push_back(blockType);
        }        
        mCurrentIndex = 0;
        std::random_shuffle(mBag.begin(), mBag.end());
    }

    
    BlockType BlockFactory::getNext() const
    {
        if (mCurrentIndex >= mBagSize * cBlockTypeCount)
        {
            // Reshuffle the bag.
            const_cast<BlockFactory*>(this)->reset();
        }
        return mBag[mCurrentIndex++];
    }


} // namespace Tetris
