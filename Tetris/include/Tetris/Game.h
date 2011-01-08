#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>


namespace Tetris {

class Block;
class GameStateNode;
class GameImpl;


/**
    * Game is a top-level class for the Tetris core. It manages the following things:
    *   - the currently active block
    *   - the list of future blocks
    *   - the root gamestate node
    *
    * It is the class that needs to be exposed to the client.
    */
class Game
{
public:

    Game(size_t inNumRows, size_t inNumColumns);

    // Indicates that a refresh is required in the higher layer view
    bool checkDirty();

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    bool move(Direction inDirection);

    bool rotate();

    void drop();

    int level() const;

    void setLevel(int inLevel);

    const Block & activeBlock() const;

    const Grid & gameGrid() const;

    void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

    void getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const;

    size_t currentBlockIndex() const;

    void appendPrecalculatedNode(NodePtr inNode);

    const GameStateNode * currentNode() const;

    const GameStateNode * lastPrecalculatedNode() const;

    bool navigateNodeDown();

    size_t numPrecalculatedMoves() const;

    void clearPrecalculatedNodes();

private:
    // non-copyable
    Game(const Game&);
    Game& operator=(const Game&);

    static std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns);
    void reserveBlocks(size_t inCount);
    void setCurrentNode(NodePtr inCurrentNode);
    void supplyBlocks() const;
    void setDirty();

    size_t mNumRows;
    size_t mNumColumns;
    NodePtr mCurrentNode;
    boost::scoped_ptr<Block> mActiveBlock;
    boost::scoped_ptr<BlockFactory> mBlockFactory;
    mutable BlockTypes mBlocks;
    size_t mCurrentBlockIndex;
    int mOverrideLevel;

    mutable bool mDirty;
    mutable boost::mutex mDirtyMutex;
};


} // namespace Tetris


#endif // GAME_H_INCLUDED
