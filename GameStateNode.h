#ifndef GAMESTATENODE_H
#define GAMESTATENODE_H


#include "GameState.h"
#include <boost/shared_ptr.hpp>
#include <set>


namespace Tetris
{

	class GameStateNode;
	typedef boost::shared_ptr<GameStateNode> ChildPtr;
	
	struct ChildPtrCompare
	{
		bool operator () (ChildPtr lhs, ChildPtr rhs);
	};
	
	class GameStateNode
	{
	public:
		GameStateNode(GameStateNode * inParent);

		const GameState & state() const;

		GameState & state();

		void setState(const GameState & inState);

		typedef std::multiset<ChildPtr, ChildPtrCompare> Children;

		Children & children();

		const Children & children() const;

		void setChildren(const Children & inChildren);

		GameStateNode * parent();

		const GameStateNode * parent() const;
		
	private:
		GameState mState;
		GameStateNode * mParent;
		Children mChildren;
		bool mDeadEnd;
	};

} // namespace Tetris

#endif // GAMESTATENODE_H
