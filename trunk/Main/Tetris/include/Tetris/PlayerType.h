#ifndef TETRIS_PLAYERTYPE_H_INCLUDED
#define TETRIS_PLAYERTYPE_H_INCLUDED


#include "Futile/Enum.h"
#include <vector>



namespace Futile {

Futile_Enum(PlayerType, 2, (Human, Computer));

} // namespace Futile


namespace Tetris {


//
// TODO: The "using" keyword should not be necessary here.
//       Try to fix this in the generator code.
//
using Futile::PlayerType;
using Futile::Human;
using Futile::Computer;
using Futile::EnumInfo;
using Futile::EnumeratorInfo;
typedef std::vector<PlayerType> PlayerTypes;


} // namespace Tetris


#endif // TETRIS_PLAYERTYPE_H_INCLUDED
