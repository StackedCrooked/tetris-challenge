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
        Game(int inNumRows, int inNumColumns);

        const Block & activeBlock() const;

        Block & activeBlock();

        // Includes the currently active block
        BlockTypes getFutureBlocks(size_t inCount) const;

        GameStateNode * currentNode();

        const GameStateNode * currentNode() const;

        void setCurrentNode(GameStateNode * inCurrentNode);

        bool isGameOver() const;

        //
        // Game commands
        //
        bool move(Direction inDirection);

        void rotate();

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
        void supplyBlocks() const;

        size_t mNumRows;
        size_t mNumColumns;
        boost::scoped_ptr<GameStateNode> mRootNode;
        GameStateNode * mCurrentNode;
        Block mActiveBlock;

        BlockFactory mBlockFactory;
        
        // All blocks, previous, current and future.
        // This list grows during the game.
        mutable BlockTypes mBlocks;
        size_t mCurrentBlockIndex;
    };

} // namespace Tetris

#endif // GAME_H_INCLUDED
