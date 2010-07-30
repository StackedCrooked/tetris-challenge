#include "Game.h"
#include "GameState.h"


namespace Tetris
{


    Game::Game(int inNumRows, int inNumColumns) :
        mRootNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))),
        mBlockFactory(cBlockTypeCount)
    {
        mCurrentNode = &mRootNode;
    }


    bool Game::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    void Game::fallCurrentBlock()
    {
        const GameState & gameState = mCurrentNode->state();

        bool canMoveDown = gameState.checkPositionValid(mActiveBlock->block(),
                                                        mActiveBlock->row() + 1,
                                                        mActiveBlock->column());
        if (canMoveDown)
        {
            mActiveBlock->moveDown();
        }
        else
        {
            // If we can't move a new block down then the game is over.
            bool gameOver = (mActiveBlock->row() == 0);
            ChildPtr child(new GameStateNode(mCurrentNode->state().commit(*mActiveBlock, gameOver)));
            mCurrentNode->children().insert(child);
            mCurrentNode = child.get();
        }
    }


    void Game::commitActiveBlock()
    {
        //const Block & nextBlock = getNextBlock(mCurrentNode->depth());
        //ChildPtr child(mRootNode.state().commit(nextBlock).release());
        //mCurrentNode->children().insert(child);
    }


} // namespace Tetris

