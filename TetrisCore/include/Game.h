#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "Block.h"
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

        bool navigateNodeUp();

        bool navigateNodeDown();

        bool navigateNodeLeft();

        bool navigateNodeRight();

        bool isGameOver() const;

        bool move(Direction inDirection);

        void rotate();

        void drop();

    private:
        GameStateNode mRootNode;
        GameStateNode * mCurrentNode;

        BlockFactory mBlockFactory;
        Block mBlock;
        std::vector<GameStateNode*> mHistory;
        std::vector<BlockType> mNextBlocks;
    };

} // namespace Tetris

#endif // GAME_H_INCLUDED
