#ifndef GAME_H
#define GAME_H


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

        void commitActiveBlock();

    private:

        GameStateNode mRootNode;
        GameStateNode * mCurrentNode;

        boost::scoped_ptr<ActiveBlock> mActiveBlock;
        BlockFactory mBlockFactory;
        std::vector<GameStateNode*> mHistory;
        std::vector<BlockType> mNextBlocks;
    };

} // namespace Tetris

#endif // GAME_H

