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
			static const int cHolePenalty = 20;
			static const int cHeightPenalty = 1;
			int result = 0;
			int numHoles = 0;

			bool foundTop = false;
			size_t top = mGrid.numRows();

			size_t numOccupiedUnderTop = 0;

			for (size_t rowIdx = 0; rowIdx != mGrid.numRows(); ++rowIdx)
			{
				for (size_t colIdx = 0; colIdx != mGrid.numColumns(); ++colIdx)
				{
					const int & value = mGrid.get(rowIdx, colIdx);
					if (value != NO_BLOCK)
					{
						if (!foundTop)
						{
							top = rowIdx;
							foundTop = true;
						}

						if (foundTop)
						{
							numOccupiedUnderTop++;
						}
					}
					else
					{
						// check for holes
						if (foundTop && rowIdx > 0)
						{
							if (mGrid.get(rowIdx - 1, colIdx) != NO_BLOCK)
							{
								numHoles++;
								result -= cHolePenalty;
								markAsDeadEnd();
								return result;
							}
						}
					}
				}
			}
			result -= (mGrid.numRows() - top) * cHeightPenalty;
			float density = (float)numOccupiedUnderTop / ((mGrid.numRows() - top) * mGrid.numColumns());
			result = (int)((float)result / density);
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