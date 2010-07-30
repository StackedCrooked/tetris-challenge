#include "Game.h"
#include "GameState.h"


namespace Tetris
{

    static std::auto_ptr<ActiveBlock> CreateCenteredActiveBlock(std::auto_ptr<Block> inNewBlock, size_t inNumColumns)
    {
        size_t middleX = static_cast<int>(0.5 + (static_cast<float>(inNumColumns - inNewBlock->grid().numColumns())/2));
        return std::auto_ptr<ActiveBlock>(new ActiveBlock(inNewBlock, 0, middleX));
    }


    Game::Game(int inNumRows, int inNumColumns) :
        mRootNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))),
        mBlockFactory(cBlockTypeCount),
        mActiveBlock(CreateCenteredActiveBlock(mBlockFactory.getNext(), inNumColumns))
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
            return;
        }

        // If we can't move a new block down then the game is over.
        bool gameOver = (mActiveBlock->row() == 0);
        ChildPtr child(new GameStateNode(mCurrentNode->state().commit(mActiveBlock, gameOver)));
        mCurrentNode->children().insert(child);
        mCurrentNode = child.get();
        if (!gameOver)
        {
            std::auto_ptr<Block> newBlock(mBlockFactory.getNext());
            size_t middleX = static_cast<int>(0.5 + (static_cast<float>(gameState.grid().numColumns() - newBlock->grid().numColumns())/2));
            mActiveBlock.reset(new ActiveBlock(newBlock, 0, middleX));
        }
    }


    void Game::autoMove()
    {
        //if (!mCurrentNode->children().empty())
        //{
        //    ChildPtr bestChild = mCurrentNode->children().begin();
        //    bestChild->
        //}
    }


    void Game::commitActiveBlock()
    {
        //const Block & nextBlock = getNextBlock(mCurrentNode->depth());
        //ChildPtr child(mRootNode.state().commit(nextBlock);
        //mCurrentNode->children().insert(child);
    }


} // namespace Tetris

