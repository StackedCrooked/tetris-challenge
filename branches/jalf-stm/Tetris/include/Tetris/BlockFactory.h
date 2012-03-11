#ifndef TETRIS_BLOCKFACTORY_H
#define TETRIS_BLOCKFACTORY_H


#include "Tetris/BlockType.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


namespace Tetris {


// The default block factory
class BlockFactory
{
public:
    // The size of the bag of blocks that shuffled and taken from.
    // @param n Determines how many times all block types appear in the bag.
    BlockFactory(unsigned n = 1);

    // Returns a random block type.
    BlockType getNext();

private:
    struct Impl;
    boost::shared_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_BLOCKFACTORY_H
