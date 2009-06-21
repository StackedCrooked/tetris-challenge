#include "PuzzleSolver.h"
#include "Parser.h"
#include <algorithm>

namespace Tetris
{

	PuzzleSolver::PuzzleSolver() :
		mRootNode(0, Block(' ', NO_BLOCK, 0), 0, 0)
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


	const GameStateNode * PuzzleSolver::rootNode() const
	{
		return &mRootNode;
	}


	void PuzzleSolver::populateNode(GameStateNode * inNode, const std::vector<Block> & inBlockTypes) const
	{
		if (!inBlockTypes.empty())
		{
			assert(inNode->children().empty());
			if (inNode->children().empty() && !inNode->isDeadEnd())
			{
				GameStateNode::Children futureGameStates;
				generateFutureGameStates(*inNode, inBlockTypes[0], futureGameStates);
				if (!futureGameStates.empty())
				{
					while (futureGameStates.size() > 2)
					{
						futureGameStates.erase(--futureGameStates.end());
					}
					inNode->setChildren(futureGameStates);
					if (inBlockTypes.size() > 2)
					{
						std::vector<Block> blockTypes;
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
					inNode->markAsDeadEnd();
				}
			}
		}
	}

	
	bool PuzzleSolver::next()
	{
		if (mNodes.size() < mBlocks.size())
		{
			std::vector<Block> blockIds;
			{
				blockIds.push_back(mBlocks[mNodes.size()]);
			}
			GameStateNode * currentNode = mNodes.empty() ? &mRootNode : mNodes.back()->get();
			populateNode(currentNode, blockIds);
			if (!currentNode->isDeadEnd() && !currentNode->children().empty())
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
		}
		return false;
	}

	
	void PuzzleSolver::tryNextBranch()
	{
		// Erase last node
		if (!mNodes.empty())
		{
			// Remove the children from this node
			mNodes.back()->get()->children().clear();

			// Mark as dead end
			mNodes.back()->get()->markAsDeadEnd();


			GameStateNode::Children::iterator & currentNodeIt = mNodes.back();			
			GameStateNode * parent = (*currentNodeIt)->parent();
			if (parent)
			{			
				assert (currentNodeIt != parent->children().end());

				// move to next branch
				++currentNodeIt; 

				// if there is no next branch, then try a level higher
				if (currentNodeIt == parent->children().end())
				{
					Nodes::iterator it = mNodes.end();
					--it;
					mNodes.erase(it);
					tryNextBranch();
				}
			}
		}
	}


	void PuzzleSolver::generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, GameStateNode::Children & outGameGrids) const
	{
		
		for (size_t rotIdx = 0; rotIdx != NumRotations(inBlock.type()); ++rotIdx)
		{
			Block block(Block(inBlock.charId(), inBlock.type(), inBlock.rotation() + rotIdx));
			
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
					ChildPtr child(new GameStateNode(&inGameStateNode, inBlock, rowIdx - 1, inColIdx));
					child->setState(inGameStateNode.state().makeGameStateWithAddedBlock(inBlock, rowIdx - 1, inColIdx));
					if (child->state().numHoles() > 0 || (rowIdx <= 2 && child->state().hasTopHoles()))
					{
						child->markAsDeadEnd();
					}
					outGameGrids.insert(child);
				}
				return;
			}
			else if (rowIdx == maxRow)
			{
				// we found the bottom of the grid => solidify
				ChildPtr child(new GameStateNode(&inGameStateNode, inBlock, rowIdx, inColIdx));
				child->setState(inGameStateNode.state().makeGameStateWithAddedBlock(inBlock, rowIdx, inColIdx));
				if (child->state().numHoles() > 0)
				{
					child->markAsDeadEnd();
				}
				outGameGrids.insert(child);
				return;
			}
		}
		assert (!"We should not come here");
	}
}