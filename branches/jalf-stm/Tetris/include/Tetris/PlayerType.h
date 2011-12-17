#ifndef TETRIS_PLAYERTYPE_H
#define TETRIS_PLAYERTYPE_H


#include <vector>


namespace Tetris {


enum PlayerType
{
    PlayerType_Human,
    PlayerType_Computer
};


typedef std::vector<PlayerType> PlayerTypes;


} // namespace Tetris


#endif // TETRIS_PLAYERTYPE_H
