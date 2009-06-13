#ifndef GAMESTATENODE_H
#define GAMESTATENODE_H

#include "GameState.h"
#include <boost/shared_ptr.hpp>
#include <set>

namespace Tetris
{
	
	struct GameStateNode
	{
		boost::shared_ptr<GameState> state;

		std::set<GameStateNode> children;
	};

	bool operator <(const GameStateNode & lhs, const GameStateNode & rhs);

} // namespace Tetris

#endif // GAMESTATENODE_H
