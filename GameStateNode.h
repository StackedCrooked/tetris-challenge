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
		GameStateNode(GameStateNode * inParent) : mParent(inParent), mDeadEnd(false) {}

		const GameState & state() const { return mState; }

		GameState & state() { return mState; }

		void setState(const GameState & inState) { mState = inState; }

		typedef std::multiset<ChildPtr, ChildPtrCompare> Children;

		Children & children() { return mChildren; }

		const Children & children() const { return mChildren; }

		void setChildren(const Children & inChildren) { mChildren = inChildren; }

		GameStateNode * parent() { return mParent; }

		const GameStateNode * parent() const { return mParent; }

		//void setParent(GameStateNode * inParent) { mParent = inParent; }
		
	private:
		GameState mState;
		GameStateNode * mParent;
		Children mChildren;
		bool mDeadEnd;
	};

} // namespace Tetris

#endif // GAMESTATENODE_H
