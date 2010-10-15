#ifndef TETRIS_BLOCKFACTORY_H_INCLUDED
#define TETRIS_BLOCKFACTORY_H_INCLUDED


#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include <vector>


namespace Tetris
{

    class Block;

    class BlockFactory
    {
    public:
        // The size of the bag of blocks that shuffled and taken from.
        BlockFactory(int inBagSize = 1);

        // Returns a random block type.
        BlockType getNext() const;

    private:
        size_t mBagSize;
        mutable size_t mCurrentIndex;
        mutable BlockTypes mBag;
    };

} // namespace Tetris


#endif // BLOCKFACTORY_H_INCLUDED
