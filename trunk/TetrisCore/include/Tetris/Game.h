#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include <memory>


namespace Tetris
{

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

        ~Game();

        std::auto_ptr<Game> clone();

        bool isGameOver() const;

        int numRows() const;

        int numColumns() const;
        
        bool move(Direction inDirection);

        bool rotate();

        void drop();

        int level() const;

        const Block & activeBlock() const;

        // Gets the currently active block and any blocks that follow.
        void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

        void getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const;

        // How many blocks have been dropped?
        size_t currentBlockIndex() const;

        //
        // For AI
        //
        GameStateNode * currentNode();

        const GameStateNode * currentNode() const;

        const GameStateNode * lastPrecalculatedNode() const;

        GameStateNode * lastPrecalculatedNode();

        bool navigateNodeDown();

        size_t numPrecalculatedMoves() const;

        void clearPrecalculatedNodes();

    private:
        // implemented for the clone() method
        Game(const Game & inGame);
        Game(std::auto_ptr<GameImpl> inImpl);

        // not allowed
        Game & operator=(const Game&);

        GameImpl * mImpl;
    };

} // namespace Tetris


#endif // GAME_H_INCLUDED
