#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


#include "Tetris/BlockFactory.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include <memory>


namespace Tetris
{

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

        ~Game();

        std::auto_ptr<Game> clone();

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

        const GameStateNode * lastPrecalculatedNode() const;

        GameStateNode * lastPrecalculatedNode();

        const GameStateNode * currentNode() const;

        //
        // Statistics
        //
        size_t currentBlockIndex() const;

        size_t numPrecalculatedMoves() const;

        void clearPrecalculatedNodes();

        //
        // Game commands
        //
        bool move(Direction inDirection);

        bool rotate();

        void drop();

        // Make the next queued game state the current game state.
        bool navigateNodeDown();

    private:
        // implemented for the clone() method
        Game(const Game & inGame);

        // not allowed
        Game & operator=(const Game&);

        GameImpl * mImpl;
    };

} // namespace Tetris


#endif // GAME_H_INCLUDED
