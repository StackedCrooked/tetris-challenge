#include "PuzzleSolver.h"
#include "Parser.h"

namespace Tetris
{

	PuzzleSolver::PuzzleSolver() :
		mRootNode(0),
		mMinNodes(100)
	{
		Parser p;
		p.parse("inputs.txt", mBlocks);
	}


	const GameStateNode * PuzzleSolver::currentNode() const
	{
		if (!mNodes.empty())
		{
			return mNodes.back()->get();
		}
		return &mRootNode;
	}


	void PuzzleSolver::populateNode(GameStateNode * inNode, const std::vector<BlockType> & inBlockTypes) const
	{
		if (!inBlockTypes.empty())
		{
			assert(inNode->children().empty());
			if (inNode->children().empty() && !inNode->state().isDeadEnd())
			{
				GameStateNode::Children futureGameStates;
				generateFutureGameStates(*inNode, inBlockTypes[0], futureGameStates);
				if (!futureGameStates.empty())
				{
					inNode->setChildren(futureGameStates);
					if (inBlockTypes.size() > 1)
					{
						std::vector<BlockType> blockTypes;
						for (size_t idx = 1; idx < inBlockTypes.size(); ++idx)
						{
							blockTypes.push_back(inBlockTypes[idx]);
						}
						GameStateNode::Children::const_iterator it = inNode->children().begin(), end = inNode->children().end();
						for (; it != end; ++it)
						{
							populateNode(it->get(), blockTypes);
						}
					}
				}
				else
				{
					inNode->state().markAsDeadEnd();
				}
			}
		}
	}

	
	bool PuzzleSolver::next()
	{
		std::vector<BlockType> blockTypes;
		if (mNodes.size() < mBlocks.size())
		{
			blockTypes.push_back(mBlocks[mNodes.size()].type);
		}
		GameStateNode * currentNode = mNodes.empty() ? &mRootNode : mNodes.back()->get();
		populateNode(currentNode, blockTypes);
		if (!currentNode->state().isDeadEnd() && !currentNode->children().empty())
		{
			GameStateNode::Children::iterator newNodeIt = currentNode->children().begin();
			mNodes.push_back(newNodeIt);
			return true;
		}
		else
		{
			tryNextBranch();
			next();
		}
		return false;
	}

	
	void PuzzleSolver::tryNextBranch()
	{
		// Erase last node
		Nodes::iterator it = mNodes.begin();
		while (it != mNodes.end())
		{
			++it;
		}
		assert (it == mNodes.end());
		mNodes.erase(--it);
		if (mNodes.size() < mMinNodes)
		{
			mMinNodes = mNodes.size();
		}

		// Try next child 
		GameStateNode::Children::iterator end = mNodes.back()->get()->parent()->children().end();
		if (++mNodes.back() == end)
		{
			tryNextBranch();
		}
	}

	
	int PuzzleSolver::depthOfOffspring(const GameStateNode * inGameStateNode, std::list<GameStateNode*> & outNodes) const
	{
		int result = 0;
		if (!inGameStateNode->children().empty())
		{
			GameStateNode * deepestNode = 0;
			GameStateNode::Children::const_iterator it = inGameStateNode->children().begin(), end = inGameStateNode->children().end();
			for (; it != end; ++it)
			{
				int depth = depthOfOffspring(it->get(), outNodes);
				if (depth > result)
				{
					result = depth;
					deepestNode = it->get();
				}
			}
			outNodes.push_front(deepestNode);
			result++;
		}
		return result;
	}


	void PuzzleSolver::generateFutureGameStates(GameStateNode & inGameStateNode, BlockType inBlockType, GameStateNode::Children & outGameGrids) const
	{
		for (size_t rotIdx = 0; rotIdx != Block::NumRotations(inBlockType); ++rotIdx)
		{
			const Block & block(Block::Get(inBlockType, rotIdx));
			size_t maxCol = inGameStateNode.state().grid().numColumns() - block.grid().numColumns();
			for (size_t colIdx = 0; colIdx <= maxCol; ++colIdx)
			{
				generateFutureGameStates(inGameStateNode, block, colIdx, outGameGrids);
			}
		}
	}


	void PuzzleSolver::generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, size_t inColIdx, GameStateNode::Children & outGameGrids) const
	{
		size_t maxRow = inGameStateNode.state().grid().numRows() - inBlock.grid().numRows();
		for (size_t rowIdx = 0; rowIdx <= maxRow; ++rowIdx)
		{
			if (!inGameStateNode.state().checkPositionValid(inBlock, rowIdx, inColIdx))
			{
				if (rowIdx > 0)
				{
					// we collided => solidify on position higher
					ChildPtr child(new GameStateNode(&inGameStateNode));
					child->setState(inGameStateNode.state().makeGameStateWithAddedBlock(inBlock, rowIdx - 1, inColIdx));
					outGameGrids.insert(child);
				}
				return;
			}
			else if (rowIdx == maxRow)
			{
				// we found the bottom of the grid => solidify
				ChildPtr child(new GameStateNode(&inGameStateNode));
				child->setState(inGameStateNode.state().makeGameStateWithAddedBlock(inBlock, rowIdx, inColIdx));
				outGameGrids.insert(child);
				return;
			}
		}
		assert (!"We should not come here");
	}
}