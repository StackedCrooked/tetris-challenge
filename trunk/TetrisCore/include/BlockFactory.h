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

        BlockType getNext() const;

    private:
        void reset();

        size_t mBagSize;
        mutable size_t mCurrentIndex;
        mutable BlockTypes mBag;
    };

} // namespace Tetris


#endif // BLOCKFACTORY_H
