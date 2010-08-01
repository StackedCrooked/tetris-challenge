#ifndef BLOCKFACTORY_H
#define BLOCKFACTORY_H


#include "Block.h"
#include <vector>


namespace Tetris
{
    
    class Block;

    class BlockFactory
    {
    public:
        // The size of the bag of blocks that shuffled and taken from.
        BlockFactory(int inBagSize = cBlockTypeCount);

        size_t numCreated() const;

        std::auto_ptr<Block> getPrevious(size_t inIndex) const;

        std::auto_ptr<Block> getNext() const;

    private:
        void reset();

        size_t mBagSize;
        mutable size_t mCurrentIndex;
        mutable std::vector<BlockType> mBag;
        mutable std::vector<BlockType> mPrevious;
    };

} // namespace Tetris



#endif // BLOCKFACTORY_H
