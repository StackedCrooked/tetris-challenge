#include "GameController.h"
#include "Game.h"
#include "GameState.h"


namespace Tetris
{


    GameController::GameController(size_t inNumRows, size_t inNumColumns) :
        mGame(new Game(inNumRows, inNumColumns))
    {
    }


    void GameController::move(Direction inDirection)
    {
        mGame->move(inDirection);
    }


    void GameController::drop()
    {
        // Simply keep moving the block down. But don't solidify yet.
        while (mGame->move(Direction_Down));
    }


    void GameController::autoMove()
    {
        //if (!mCurrentNode->children().empty())
        //{
        //    ChildPtr bestChild = mCurrentNode->children().begin();
        //    bestChild->
        //}
    }


} // namespace Tetris
