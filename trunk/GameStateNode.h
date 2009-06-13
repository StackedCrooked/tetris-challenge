#ifndef GAMESTATENODE_H
#define GAMESTATENODE_H

#include "GameState.h"
#include <boost/shared_ptr.hpp>
#include <set>

namespace Tetris
{

	struct GameStateNode;
	typedef boost::shared_ptr<GameStateNode> ChildPtr;
	
	struct ChildPtrCompare
	{
		bool operator () (ChildPtr lhs, ChildPtr rhs);
	};
	
	struct GameStateNode
	{
		GameState state;

		typedef std::set<ChildPtr, ChildPtrCompare> Children;
		Children children;
	};

} // namespace Tetris

#endif // GAMESTATENODE_H
