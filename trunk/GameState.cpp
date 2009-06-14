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
		mCachedScore(0),
		mDeadEnd(false)
	{
	}


	const Grid & GameState::grid() const
	{
		return mGrid;
	}


	Grid & GameState::grid()
	{
		return mGrid;
	}


	int GameState::calculateScore() const
	{
		if (mDirty)
		{
			static const int cHolePenalty = 10;
			static const int cHeightPenalty = 1;
			int result = 0;
			int numHoles = 0;
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
						top = std::min<size_t>(top, rowIdx);
						if (!foundColTop)
						{
							colTop = rowIdx;
							foundColTop = true;
						}
					}
					else
					{
						if (foundColTop)
						{
							numHoles++;
							// we found a hole in the grid
							result -= cHolePenalty;
							if (numHoles >= 1)
							{
								markAsDeadEnd();
							}
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

	
	GameState GameState::makeGameStateWithAddedBlock(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
	{
		GameState result(*this);
		result.mDirty = true;
		for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
		{
			for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
			{
				if (inBlock.grid().get(r, c) != NO_BLOCK)
				{
					result.grid().set(inRowIdx + r, inColIdx + c, inBlock.type());
				}
			}
		}
		return result;
	}

} // namespace Tetris