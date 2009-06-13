#include "GameStateNode.h"

namespace Tetris
{	

	bool operator <(const GameStateNode & lhs, const GameStateNode & rhs)
	{
		return (*lhs.state) < (*rhs.state);
	}

} // namespace Tetris