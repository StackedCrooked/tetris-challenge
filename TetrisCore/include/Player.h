#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include <vector>


namespace Tetris
{

    class Player
    {
    public:
        Player(Game * inGame);

        void move(const std::vector<int> & inDepths);

    private:
        Game * mGame;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
