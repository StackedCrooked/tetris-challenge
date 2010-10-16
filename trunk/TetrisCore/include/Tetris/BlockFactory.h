#ifndef TETRIS_BLOCKFACTORY_H_INCLUDED
#define TETRIS_BLOCKFACTORY_H_INCLUDED


#include "Tetris/Enum.h"


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

        ~BlockFactory();

        // Returns a random block type.
        BlockType getNext() const;

    private:
        BlockFactory(const BlockFactory &);
        BlockFactory& operator=(const BlockFactory&);

        BlockFactoryImpl * mImpl;
    };

} // namespace Tetris


#endif // BLOCKFACTORY_H_INCLUDED
