#include "PuzzleSolver.h"
#include "Parser.h"

namespace Tetris
{

	PuzzleSolver::PuzzleSolver()
	{
		Parser p;
		p.parse("inputs.txt", mBlocks);
	}


	void PuzzleSolver::start()
	{
		for (size_t idx = 0; idx != mBlocks.size(); ++idx)
		{
			GameState gg;
			std::set<GameState> futureGameStates;

			for (size_t rotIdx = 0; rotIdx != Block::NumRotations(mBlocks[idx].type); ++rotIdx)
			{
				generateFutureGameStates(gg, Block::Get(mBlocks[idx].type, rotIdx), futureGameStates);
			}
			if (!futureGameStates.empty())
			{
				mGameState = *futureGameStates.begin();
			}
		}	
	}


	void PuzzleSolver::generateFutureGameStates(const GameState & inGameState, const Block & inBlock, std::set<GameState> & outGameGrids)
	{
		size_t maxCol = inGameState.grid().numColumns() - inBlock.grid().numColumns();
		for (size_t colIdx = 0; colIdx <= maxCol; ++colIdx)
		{
			generateFutureGameStates(inGameState, inBlock, colIdx, outGameGrids);
		}
	}


	void PuzzleSolver::generateFutureGameStates(const GameState & inGameState, const Block & inBlock, size_t inColIdx, std::set<GameState> & outGameGrids) const
	{
		size_t maxRow = inGameState.grid().numRows() - inBlock.grid().numRows();
		for (size_t rowIdx = 0; rowIdx <= maxRow; ++rowIdx)
		{
			if (!inGameState.checkPositionValid(inBlock, rowIdx, inColIdx))
			{
				if (rowIdx > 0)
				{
					// we collided => solidify on position higher
					outGameGrids.insert(inGameState.makeGameStateWithAddedBlock(inBlock, rowIdx - 1, inColIdx));
				}
				return;
			}
			else if (rowIdx == maxRow)
			{
				// we found the bottom of the grid => solidify
				outGameGrids.insert(inGameState.makeGameStateWithAddedBlock(inBlock, rowIdx, inColIdx));
				return;
			}
		}
		assert (!"We should not come here");
	}
}