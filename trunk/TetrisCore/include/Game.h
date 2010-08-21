#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "Block.h"
#include "BlockFactory.h"
#include "Direction.h"
#include "GameStateNode.h"
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
    class Game
    {
    public:
        Game(int inNumRows = 20, int inNumColumns = 10, const BlockTypes & inBlocks = BlockTypes());

        std::auto_ptr<Game> clone() const;

        int numRows() const;

        int numColumns() const;

        void reserveBlocks(size_t inCount);

        const Block & activeBlock() const;

        Block & activeBlock();

        // Includes the currently active block
        void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

        GameStateNode * currentNode();

        const GameStateNode * currentNode() const;

        bool isGameOver() const;

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

        //
        // Navigate the game history and alternative histories.
        // Experimental.
        //
        bool navigateNodeUp();

        bool navigateNodeDown();

        bool navigateNodeLeft();

        bool navigateNodeRight();

    private:
        Game(const Game & inGame);

        void setCurrentNode(GameStateNode * inCurrentNode);

        void supplyBlocks() const;

        size_t mNumRows;
        size_t mNumColumns;
        boost::scoped_ptr<GameStateNode> mRootNode;
        GameStateNode * mCurrentNode;
        boost::scoped_ptr<Block> mActiveBlock;

        BlockFactory mBlockFactory;
        
        // All blocks, previous, current and future.
        // This list grows during the game.
        mutable BlockTypes mBlocks;
        size_t mCurrentBlockIndex;
    };

} // namespace Tetris

#endif // GAME_H_INCLUDED
