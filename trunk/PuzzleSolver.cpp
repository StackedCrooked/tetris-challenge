#include "PuzzleSolver.h"
#include "Parser.h"

namespace Tetris
{

	PuzzleSolver::PuzzleSolver() :
		mTreeDepth(0)
	{
		mCurrentNode = &mRootNode;
		Parser p;
		p.parse("inputs.txt", mBlocks);
	}

	
	bool PuzzleSolver::next()
	{
		if (mTreeDepth < mBlocks.size())
		{
			GameStateNode::Children futureGameStates;

			BlockType blockType = mBlocks[mTreeDepth].type;
			for (size_t rotIdx = 0; rotIdx != Block::NumRotations(blockType); ++rotIdx)
			{
				generateFutureGameStates(mCurrentNode->state, Block::Get(blockType, rotIdx), futureGameStates);
			}
			mCurrentNode->children = futureGameStates;
			if (!mCurrentNode->children.empty())
			{
				GameStateNode * parentNode = mCurrentNode;
				mCurrentNode = (mCurrentNode->children.begin())->get();
				mCurrentNode->parent = parentNode;
			}
			mTreeDepth++;
			return true;
		}
		return false;
	}


	void PuzzleSolver::generateFutureGameStates(const GameState & inGameState, const Block & inBlock, GameStateNode::Children & outGameGrids)
	{
		size_t maxCol = inGameState.grid().numColumns() - inBlock.grid().numColumns();
		for (size_t colIdx = 0; colIdx <= maxCol; ++colIdx)
		{
			generateFutureGameStates(inGameState, inBlock, colIdx, outGameGrids);
		}
	}


	void PuzzleSolver::generateFutureGameStates(const GameState & inGameState, const Block & inBlock, size_t inColIdx, GameStateNode::Children & outGameGrids) const
	{
		size_t maxRow = inGameState.grid().numRows() - inBlock.grid().numRows();
		for (size_t rowIdx = 0; rowIdx <= maxRow; ++rowIdx)
		{
			if (!inGameState.checkPositionValid(inBlock, rowIdx, inColIdx))
			{
				if (rowIdx > 0)
				{
					// we collided => solidify on position higher
					ChildPtr child(new GameStateNode);
					child->state = inGameState.makeGameStateWithAddedBlock(inBlock, rowIdx - 1, inColIdx);
					outGameGrids.insert(child);
				}
				return;
			}
			else if (rowIdx == maxRow)
			{
				// we found the bottom of the grid => solidify
				ChildPtr child(new GameStateNode);
				child->state = inGameState.makeGameStateWithAddedBlock(inBlock, rowIdx, inColIdx);
				outGameGrids.insert(child);
				return;
			}
		}
		assert (!"We should not come here");
	}
}