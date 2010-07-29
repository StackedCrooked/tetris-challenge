#include "Game.h"
#include "GameState.h"


namespace Tetris
{


    Game::Game(int inNumRows, int inNumColumns) :
        mRootNode(new GameState(inNumRows, inNumColumns))
    {
        mCurrentNode = &mRootNode;
    }


    void Game::tick()
    {
        //mRootNode->tick();
    }


    const Block & getNextBlock(int inDepth)
    {
        static Block block(BlockType_J, 0);
        return block;
    }


    void Game::commitActiveBlock()
    {
        //const Block & nextBlock = getNextBlock(mCurrentNode->depth());
        //ChildPtr child(mRootNode.state().commit(nextBlock).release());
        //mCurrentNode->children().insert(child);
    }


} // namespace Tetris

