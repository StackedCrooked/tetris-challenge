#ifndef TETRIS_BLOCKFACTORY_H
#define TETRIS_BLOCKFACTORY_H


#include "Tetris/BlockType.h"
#include "Futile/Threading.h"
#include <boost/noncopyable.hpp>
#include <memory>


namespace Tetris {


// The default block factory
class BlockFactory : boost::noncopyable
{
public:
    // The size of the bag of blocks that shuffled and taken from.
    // @param n Determines how many times all block types appear in the bag.
    BlockFactory(unsigned n = 1);

    virtual ~BlockFactory();

    // Returns a random block type.
    virtual BlockType getNext();

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_BLOCKFACTORY_H
