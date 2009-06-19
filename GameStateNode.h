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

		const GameStateNode * parent() const;	

		GameStateNode * parent();

		typedef std::multiset<ChildPtr, ChildPtrCompare> Children;

		Children & children();

		const Children & children() const;

		void setChildren(const Children & inChildren);
		
		int depthOfOffspring() const;

		const GameState & state() const;

		void setState(const GameState & inState);

		bool isDeadEnd() const { return mDeadEnd; }	

		void markAsDeadEnd() { mDeadEnd = true; }
		
	private:
		GameStateNode * mParent;
		bool mDeadEnd;
		GameState mState;
		Children mChildren;
	};

} // namespace Tetris

#endif // GAMESTATENODE_H
