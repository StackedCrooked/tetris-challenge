#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "Tetris/Block.h"
#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/Direction.h"
#include "Tetris/GameStateNode.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <memory>
#include <vector>


namespace Tetris
{

    /**
     * Game is a top-level class for the Tetris core. It manages the following things:
     *   - the currently active block
     *   - the list of future blocks
     *   - the root gamestate node
     *
     * It is the class that needs to be exposed to the client.
     */
    class Game : boost::noncopyable
    {
    public:
        Game(size_t inNumRows, size_t inNumColumns);

        ~Game();

        int numRows() const;

        int numColumns() const;

        void reserveBlocks(size_t inCount);

        const Block & activeBlock() const;

        Block & activeBlock();

        // Includes the currently active block
        void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

        void getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const;

        bool isGameOver() const;

        GameStateNode * currentNode();

        const GameStateNode * endNode() const;

        inline GameStateNode * endNode()
        { return const_cast<GameStateNode*>(static_cast<const Game *>(this)->endNode()); }

        const GameStateNode * currentNode() const;

        //
        // Statistics
        //
        size_t currentBlockIndex() const
        { return mCurrentBlockIndex; }

        size_t numPrecalculatedMoves() const;

        //
        // Game commands
        //
        bool move(Direction inDirection);

        bool rotate();

        void drop();

        // Make the next queued game state the current game state.
        bool navigateNodeDown();

    private:
        void setCurrentNode(NodePtr inCurrentNode);

        void supplyBlocks() const;

        size_t mNumRows;
        size_t mNumColumns;
        NodePtr mCurrentNode;
        boost::scoped_ptr<Block> mActiveBlock;
        boost::scoped_ptr<BlockFactory> mBlockFactory;
        mutable BlockTypes mBlocks;
        size_t mCurrentBlockIndex;
    };

} // namespace Tetris


#endif // GAME_H_INCLUDED
