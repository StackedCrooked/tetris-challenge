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
        switch (inDirection)
        {
            case Direction_Up:
            {
                
                break;
            }
            case Direction_Down:
            {
                break;
            }
            case Direction_Left:
            {
                break;
            }
            case Direction_Right:
            {
                break;
            }
            default:
            {
                throw std::logic_error("Invalid enum value");
            }
        }
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

