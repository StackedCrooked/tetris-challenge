#include "Player.h"


namespace Tetris
{


    Player::Player(Game * inGame) :
        mGame(inGame)
    {
    }


    void Player::move(const std::vector<int> & inDepths)
    {
        GameStateNode & node = mGame->currentNode();
        GameState & state = node.state();
        
        
    }


} // namespace Tetris

