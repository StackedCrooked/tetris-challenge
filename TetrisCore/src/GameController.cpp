#include "GameController.h"
#include "Game.h"
#include "GameState.h"


namespace Tetris
{


    GameController::GameController(size_t inNumRows, size_t inNumColumns) :
        mGame(new Game(inNumRows, inNumColumns))
    {
    }


    Game & GameController::game()
    {
        return *mGame;
    }


    const Game & GameController::game() const
    {
        return *mGame;
    }


    void GameController::move(Direction inDirection)
    {
        mGame->move(inDirection);
    }


    void GameController::rotate()
    {
        mGame->rotate();
    }


    void GameController::drop()
    {
        // Simply keep moving the block down. But don't solidify yet.
        while (mGame->move(Direction_Down));
    }


} // namespace Tetris
