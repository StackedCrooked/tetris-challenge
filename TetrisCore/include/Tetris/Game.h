#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "Tetris/Block.h"
#include "Tetris/BlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/Direction.h"
#include "Tetris/GameStateNode.h"
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
        Game(size_t inNumRows, size_t inNumColumns);

        ~Game();

        std::auto_ptr<Game> clone() const;

        int numRows() const;

        int numColumns() const;

        void reserveBlocks(size_t inCount);

        const Block & activeBlock() const;

        Block & activeBlock();

        // Includes the currently active block
        void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

        bool isGameOver() const;

        GameStateNode * currentNode();

        const GameStateNode * endNode() const;

        inline GameStateNode * endNode()
        { return const_cast<GameStateNode*>(static_cast<const Game *>(this)->endNode()); }

        const GameStateNode * currentNode() const;

        // Removes all nodes that came before the current node.
        // This function is mainly used to prevent a stack overflow
        // when destructing a Game object that has a very deep node tree.
        void eraseHistory();

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
        // Implemented
        explicit Game(const Game & inGame);

        // Not implemented
        Game & operator&();

        void setCurrentNode(const GameStateNode * inCurrentNode);

        void supplyBlocks() const;

        size_t mNumRows;
        size_t mNumColumns;
        boost::scoped_ptr<GameStateNode> mRootNode;
        GameStateNode * mCurrentNode;
        boost::scoped_ptr<Block> mActiveBlock;
        boost::scoped_ptr<BlockFactory> mBlockFactory;
        mutable BlockTypes mBlocks;
        size_t mCurrentBlockIndex;
    };

} // namespace Tetris

#endif // GAME_H_INCLUDED
