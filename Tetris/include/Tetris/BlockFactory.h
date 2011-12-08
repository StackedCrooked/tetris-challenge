#ifndef TETRIS_BLOCKFACTORY_H_INCLUDED
#define TETRIS_BLOCKFACTORY_H_INCLUDED


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
    BlockFactory(int inBagSize = 1);

    virtual ~BlockFactory();

    // Returns a random block type.
    virtual BlockType getNext() const;

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // BLOCKFACTORY_H_INCLUDED
