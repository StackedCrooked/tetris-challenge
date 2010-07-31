#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "BlockFactory.h"
#include "Direction.h"
#include "GameStateNode.h"
#include <memory>
#include <vector>


namespace Tetris
{

    class Game
    {
    public:
        Game(int inNumRows, int inNumColumns);

        const Block & activeBlock() const;

        Block & activeBlock();

        GameStateNode & currentNode();

        const GameStateNode & currentNode() const;

        bool isGameOver() const;

        bool move(Direction inDirection);

        void rotate();

        // Drops the active block one square
        void moveDown();

    private:

        GameStateNode mRootNode;
        GameStateNode * mCurrentNode;

        BlockFactory mBlockFactory;
        std::auto_ptr<Block> mBlock;
        std::vector<GameStateNode*> mHistory;
        std::vector<BlockType> mNextBlocks;
    };

} // namespace Tetris

#endif // GAME_H_INCLUDED
