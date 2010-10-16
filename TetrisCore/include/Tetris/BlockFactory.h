#ifndef TETRIS_BLOCKFACTORY_H_INCLUDED
#define TETRIS_BLOCKFACTORY_H_INCLUDED


#include "Tetris/BlockType.h"
#include <boost/scoped_ptr.hpp>


namespace Tetris
{
 
    class Block;    
    class BlockFactoryImpl;
    Tetris_DeclareEnum(BlockType);
    

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
