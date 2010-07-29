#ifndef GAME_H
#define GAME_H


#include "Block.h"
#include "GameStateNode.h"


namespace Tetris
{

    // This is the model, not the controller
    class Game
    {
    public:
        Game(int inNumRows, int inNumColumns);

        // Drops the active block one square
        void tick();

        void commitActiveBlock();

    private:
        const Block & getNextBlock(int inDepth);

        GameStateNode mRootNode;
        GameStateNode * mCurrentNode;
        std::vector<BlockType> mNextBlocks;
    };

} // namespace Tetris

#endif // GAME_H

