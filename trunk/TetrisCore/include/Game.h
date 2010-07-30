#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "Block.h"
#include "BlockFactory.h"
#include "GameStateNode.h"


namespace Tetris
{

    class Game
    {
    public:
        Game(int inNumRows, int inNumColumns);

        bool isGameOver() const;

        // Drops the active block one square
        void fallCurrentBlock();

        void autoMove();

        void commitActiveBlock();

    private:

        GameStateNode mRootNode;
        GameStateNode * mCurrentNode;

        BlockFactory mBlockFactory;
        std::auto_ptr<ActiveBlock> mActiveBlock;
        std::vector<GameStateNode*> mHistory;
        std::vector<BlockType> mNextBlocks;
    };

} // namespace Tetris

#endif // GAME_H_INCLUDED
