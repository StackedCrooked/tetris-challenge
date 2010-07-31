#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "BlockFactory.h"
#include "Direction.h"
#include "GameStateNode.h"
#include <memory>
#include <vector>


namespace Tetris
{

    class ActiveBlock;

    class Game
    {
    public:
        Game(int inNumRows, int inNumColumns);

        const ActiveBlock & activeBlock() const;

        ActiveBlock & activeBlock();

        GameStateNode & currentNode();

        const GameStateNode & currentNode() const;

        bool isGameOver() const;

        bool move(Direction inDirection);

        // Drops the active block one square
        void moveDown();

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
