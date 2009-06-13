#include "GameState.h"
#include <assert.h>
#include <windows.h>


namespace Tetris
{

	const int cNumRows = 15;
	const int cNumColumns = 15;


	bool operator<(const GameState & lhs, const GameState & rhs)
	{
		// ordering is by score descending
		return lhs.calculateScore() > rhs.calculateScore();
	}


	GameState::GameState() :
		mGrid(cNumRows, cNumColumns, NO_BLOCK),
		mDirty(true),
		mCachedScore(0)
	{
	}


	const GenericGrid<int> & GameState::grid() const
	{
		return mGrid;
	}


	bool GameState::addBlock(const Block & inBlock, std::set<GameState> & outGameGrids)
	{
		size_t maxCol = mGrid.numColumns() - inBlock.grid().numColumns();
		for (size_t colIdx = 0; colIdx <= maxCol; ++colIdx)
		{
			addBlock(inBlock, colIdx, outGameGrids);
		}
		// we can't insert the block in the grid at all
		// so return false
		return false;
	}


	bool GameState::addBlock(const Block & inBlock, size_t inColIdx, std::set<GameState> & outGameGrids) const
	{
		size_t maxRow = mGrid.numRows() - inBlock.grid().numRows();
		for (size_t rowIdx = 0; rowIdx <= maxRow; ++rowIdx)
		{
			if (!checkPositionValid(inBlock, rowIdx, inColIdx))
			{
				if (rowIdx > 0)
				{
					// we collided => solidify on position higher
					GameState gg(*this);
					gg.solidifyBlockPosition(inBlock, rowIdx - 1, inColIdx);
					outGameGrids.insert(gg);
					return true;
				}
				else
				{			
					// we're stuck at the top => return
					return false;
				}
			}
			else
			{
				if (rowIdx == maxRow)
				{
					// we found the bottom of the grid => solidify
					GameState gg(*this);
					gg.solidifyBlockPosition(inBlock, rowIdx, inColIdx);
					outGameGrids.insert(gg);
					return true;
				}
			}
		}
		assert (!"We should not come here");
		return false;
	}


	int GameState::calculateScore() const
	{
		if (mDirty)
		{
			static const int cHolePenalty = 1;
			static const int cHeightPenalty = 1;
			int result = 0;
			bool foundTop = false;
			size_t top = mGrid.numRows();
			for (size_t colIdx = 0; colIdx != mGrid.numColumns(); ++colIdx)
			{
				bool foundColTop = false;
				size_t colTop = mGrid.numRows();
				for (size_t rowIdx = 0; rowIdx != mGrid.numRows(); ++rowIdx)
				{
					const int & value = mGrid.get(rowIdx, colIdx);
					if (value != NO_BLOCK)
					{
						if (!foundTop)
						{
							top = rowIdx;
							foundTop = true;
						}

						if (!foundColTop)
						{
							colTop = rowIdx;
							result -= static_cast<int>(mGrid.numRows() - colTop) * cHeightPenalty;
							foundColTop = true;
						}
					}
					else
					{
						if (foundColTop)
						{
							// we found a hole in the grid
							result -= cHolePenalty;
						}
					}
				}
			}
			result -= static_cast<int>(mGrid.numRows() - top) * cHeightPenalty;
			mCachedScore = result;
			mDirty = false;
		}
		return mCachedScore;
	}


	bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
	{
		const size_t maxRows = mGrid.numRows() - inBlock.grid().numRows();
		const size_t maxCols = mGrid.numColumns() - inBlock.grid().numColumns();
		for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
		{
			for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
			{
				if (inBlock.grid().get(r, c) != 0 && mGrid.get(inRowIdx + r, inColIdx + c) != NO_BLOCK)
				{
					return false;
				}
			}
		}
		return true;
	}


	void GameState::solidifyBlockPosition(const Block & inBlock, size_t inRowIdx, size_t inColIdx)
	{
		for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
		{
			for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
			{
				if (inBlock.grid().get(r, c) != 0)
				{
					mGrid.set(inRowIdx + r, inColIdx + c, inBlock.type());
				}
			}
		}
		mDirty = true;
	}

} // namespace Tetris