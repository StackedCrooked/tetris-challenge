#ifndef TETRIS_BLOCKFACTORY_H_INCLUDED
#define TETRIS_BLOCKFACTORY_H_INCLUDED


#include <memory>


namespace Tetris {


class Block;
class BlockFactoryImpl;
typedef char BlockType;


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

    BlockFactoryImpl * mImpl;
};


} // namespace Tetris


#endif // BLOCKFACTORY_H_INCLUDED