#ifndef TETRIS_BLOCKFACTORY_H_INCLUDED
#define TETRIS_BLOCKFACTORY_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Futile/Threading.h"
#include <memory>


namespace Tetris {


class Block;


class AbstractBlockFactory
{
public:
    AbstractBlockFactory();

    virtual ~AbstractBlockFactory() = 0;

    // Returns a random block type.
    virtual BlockType getNext() const = 0;

private:
    AbstractBlockFactory(const AbstractBlockFactory &);
    AbstractBlockFactory& operator=(const AbstractBlockFactory&);
};


// The default block factory
class BlockFactory : public AbstractBlockFactory
{
public:
    // The size of the bag of blocks that shuffled and taken from.
    BlockFactory(int inBagSize = 1);

    virtual ~BlockFactory();

    // Returns a random block type.
    virtual BlockType getNext() const;

private:
    BlockFactory(const BlockFactory &);
    BlockFactory& operator=(const BlockFactory&);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // BLOCKFACTORY_H_INCLUDED
