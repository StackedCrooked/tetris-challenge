#include "GameState.h"
#include <assert.h>


namespace Tetris
{

	const int cNumRows = 15;
	const int cNumColumns = 15;


	int GameState::sMarker(1);
	GenericGrid<int> GameState::sHelperGrid(cNumRows, cNumColumns, 0);


	bool operator<(const GameState & lhs, const GameState & rhs)
	{
		// ordering is by score descending
		return lhs.calculateScore() > rhs.calculateScore();
	}


	GameState::GameState() :
		mGrid(cNumRows, cNumColumns, NO_BLOCK),
		mDirty(true),
		mCachedScore(0),
		mNumHoles(0)
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


	bool GameState::hasTopHoles() const
	{
		for (size_t colIdx = 0; colIdx != mGrid.numColumns(); ++colIdx)
		{
			if (mGrid.get(0, colIdx) == NO_BLOCK)
			{
				sMarker++;
				int numNeighbors = countNeighbors(0, colIdx);
				if (numNeighbors == 1  || numNeighbors == 2 ||
					numNeighbors == 5  || numNeighbors == 6 ||
					numNeighbors == 9  || numNeighbors == 10 ||
					numNeighbors == 13 || numNeighbors == 14 ||
					numNeighbors == 17 || numNeighbors == 18 ||
					numNeighbors == 21 || numNeighbors == 22 ||
					numNeighbors == 25 || numNeighbors == 26 ||
					numNeighbors == 29 || numNeighbors == 30 ||
					numNeighbors == 33 || numNeighbors == 34 ||
					numNeighbors == 37 || numNeighbors == 38)
				{
					return true;
				}
			}
		}
		return false;
	}


	int GameState::countNeighbors(size_t inRowIdx, size_t inColIdx) const
	{
		int result = 0;
		sHelperGrid.set(inRowIdx, inColIdx, sMarker);
		int value = mGrid.get(inRowIdx, inColIdx);

		if (inRowIdx + 1 < mGrid.numRows() &&
			mGrid.get(inRowIdx + 1, inColIdx) == value &&
			sHelperGrid.get(inRowIdx + 1, inColIdx) != sMarker)
		{
			result++;
			result += countNeighbors(inRowIdx + 1, inColIdx);
		}
		
		if (inColIdx + 1 < mGrid.numColumns() &&
			mGrid.get(inRowIdx, inColIdx + 1) == value &&
			sHelperGrid.get(inRowIdx, inColIdx + 1) != sMarker)
		{
			result++;
			result += countNeighbors(inRowIdx, inColIdx + 1);
		}

		if (inRowIdx > 0 &&
			mGrid.get(inRowIdx - 1, inColIdx) == value &&
			sHelperGrid.get(inRowIdx - 1, inColIdx) != sMarker)
		{
			result++;
			result += countNeighbors(inRowIdx - 1, inColIdx);
		}
		
		if (inColIdx > 0 &&
			mGrid.get(inRowIdx, inColIdx - 1) == value &&
			sHelperGrid.get(inRowIdx, inColIdx - 1) != sMarker)
		{
			result++;
			result += countNeighbors(inRowIdx, inColIdx - 1);
		}

		return result;
	}


	int GameState::calculateScore() const
	{
		if (mDirty)
		{
			static const int cHolePenalty = 2;
			static const int cHeightPenalty = 1;
			int result = 0;
			mNumHoles = 0;

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
								mNumHoles++;
								result -= cHolePenalty;
							}
						}
					}
				}
			}

			result -= (mGrid.numRows() - top) * cHeightPenalty;
			float density = ((float)numOccupiedUnderTop) / (float) (((mGrid.numRows() - top) * mGrid.numColumns()));
			result += static_cast<int>((11.0*density) + 0.5);
			mCachedScore = result;
			mDirty = false;
		}
		return mCachedScore;
	}


	int GameState::numHoles() const
	{
		return mNumHoles;
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
