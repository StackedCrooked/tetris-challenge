#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
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

        std::auto_ptr<Game> clone() const;

        bool isGameOver() const;

        int rowCount() const;

        int columnCount() const;
        
        bool move(Direction inDirection);

        bool rotate();

        void drop();

        int level() const;

        // Set to -1 to revert to default level calculation (#lines/10).
        void setLevel(int inLevel);

        const Block & activeBlock() const;

        const Grid & gameGrid() const;

        // Gets the currently active block and any blocks that follow.
        void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

        // Offset 0 retrieves the first block.
        // Use offset currentBlocIndex() + 1 to get futures blocks.
        void getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const;

        // How many blocks have been dropped?
        size_t currentBlockIndex() const;

        //
        // For AI
        //
        const GameStateNode * currentNode() const;

        const GameStateNode * lastPrecalculatedNode() const;

        void appendPrecalculatedNode(NodePtr inNode);

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