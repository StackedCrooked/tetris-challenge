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
		GameStateNode() : parent(0) {}

		GameState state;

		typedef std::multiset<ChildPtr, ChildPtrCompare> Children;
		Children children;

		GameStateNode * parent;
	};

} // namespace Tetris

#endif // GAMESTATENODE_H
