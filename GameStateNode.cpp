#include "GameStateNode.h"

namespace Tetris
{

	bool ChildPtrCompare::operator () (ChildPtr lhs, ChildPtr rhs)
	{
		if (lhs && rhs)
		{
			return lhs->state() < rhs->state();
		}
		else
		{
			return lhs.get() < rhs.get();
		}
	}
	
	
	GameStateNode::GameStateNode(GameStateNode * inParent, const Block & inBlock, size_t inRowIdx, size_t inColIdx) :
		mParent(inParent),
		mLastBlock(inBlock),
		mRowIdx(inRowIdx),
		mColIdx(inColIdx),
		mDeadEnd(false)
	{
	}


	const GameState & GameStateNode::state() const
	{
		return mState;
	}
	
	
	void GameStateNode::setState(const GameState & inState)
	{
		mState = inState;
	}

	
	GameStateNode::Children & GameStateNode::children()
	{
		return mChildren;
	}
	
	
	const GameStateNode::Children & GameStateNode::children() const
	{
		return mChildren;
	}
	
	
	void GameStateNode::setChildren(const Children & inChildren)
	{
		mChildren = inChildren;
	}

	
	GameStateNode * GameStateNode::parent()
	{
		return mParent;
	}

	
	const GameStateNode * GameStateNode::parent() const
	{
		return mParent;
	}
	
	
	const Block & GameStateNode::lastBlock() const
	{
		return mLastBlock;
	}

	
	void GameStateNode::lastBlockPosition(size_t & outRowIdx, size_t &outColIdx) const
	{
		outRowIdx = mRowIdx;
		outColIdx = mColIdx;
	}

	
	int GameStateNode::depthOfOffspring() const
	{
		int result = 0;
		if (!children().empty())
		{
			result++;
			GameStateNode * deepestNode = 0;
			GameStateNode::Children::const_iterator it = children().begin(), end = children().end();
			int deepestChildOffspring = 0;
			for (; it != end; ++it)
			{
				int childOffspring = it->get()->depthOfOffspring();
				if (childOffspring > deepestChildOffspring)
				{
					deepestChildOffspring = childOffspring;
				}
			}
			result += deepestChildOffspring;
		}
		return result;
	}

} // namespace Tetris
