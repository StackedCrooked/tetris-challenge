#include "GameController.h"
#include "GameState.h"


namespace Tetris
{

    //static std::auto_ptr<ActiveBlock> CreateCenteredActiveBlock(std::auto_ptr<Block> inNewBlock, size_t inNumColumns)
    //{
    //    size_t middleX = static_cast<int>(0.5 + (static_cast<float>(inNumColumns - inNewBlock->grid().numColumns())/2));
    //    return std::auto_ptr<ActiveBlock>(new ActiveBlock(inNewBlock, 0, middleX));
    //}


    //GameController::GameController(int inNumRows, int inNumColumns) :
    //    mRootNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))),
    //    mBlockFactory(cBlockTypeCount),
    //    mActiveBlock(CreateCenteredActiveBlock(mBlockFactory.getNext(), inNumColumns))
    //{
    //    mCurrentNode = &mRootNode;
    //}


    //bool GameController::isGameOver() const
    //{
    //    return mCurrentNode->state().isGameOver();
    //}


    //void GameController::fallCurrentBlock()
    //{
    //    const GameState & gameState = mCurrentNode->state();

    //    bool canMoveDown = gameState.checkPositionValid(mActiveBlock->block(),
    //                                                    mActiveBlock->row() + 1,
    //                                                    mActiveBlock->column());
    //    if (canMoveDown)
    //    {
    //        mActiveBlock->moveDown();
    //        return;
    //    }

    //    // If we can't move a new block down then the game is over.
    //    bool gameOver = (mActiveBlock->row() == 0);
    //    ChildPtr child(new GameStateNode(mCurrentNode->state().commit(mActiveBlock, gameOver)));
    //    mCurrentNode->children().insert(child);
    //    mCurrentNode = child.get();
    //    if (!gameOver)
    //    {
    //        mActiveBlock.reset(CreateCenteredActiveBlock(mBlockFactory.getNext(), gameState.grid().numColumns()));
    //    }
    //}


    //void GameController::autoMove()
    //{
    //    //if (!mCurrentNode->children().empty())
    //    //{
    //    //    ChildPtr bestChild = mCurrentNode->children().begin();
    //    //    bestChild->
    //    //}
    //}


    //void GameController::commitActiveBlock()
    //{
    //    //const Block & nextBlock = getNextBlock(mCurrentNode->depth());
    //    //ChildPtr child(mRootNode.state().commit(nextBlock);
    //    //mCurrentNode->children().insert(child);
    //}


} // namespace Tetris

