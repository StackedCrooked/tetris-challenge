#ifndef TETRIS_PLAYERTYPE_H_INCLUDED
#define TETRIS_PLAYERTYPE_H_INCLUDED


#include "Futile/Enum.h"
#include <vector>


namespace Tetris {


Futile_Enum(PlayerType, 2, (Human, Computer));
typedef std::vector<PlayerType> PlayerTypes;


} // namespace Tetris


#endif // TETRIS_PLAYERTYPE_H_INCLUDED
