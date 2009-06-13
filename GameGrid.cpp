#include "GameGrid.h"
#include <assert.h>
#include <windows.h>

const int cNumRows = 15;
const int cNumColumns = 15;


bool operator<(const GameGrid & lhs, const GameGrid & rhs)
{
	// ordering is by score descending
	return lhs.calculateScore() > rhs.calculateScore();
}


GameGrid::GameGrid() :
	mGrid(cNumRows, cNumColumns, Block::MAX_BLOCK),
	mDirty(true),
	mCachedScore(0)
{
}


const GenericGrid<int> & GameGrid::grid() const
{
	return mGrid;
}


bool GameGrid::addBlock(const Block & inBlock, std::set<GameGrid> & outGameGrids)
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


bool GameGrid::addBlock(const Block & inBlock, size_t inColIdx, std::set<GameGrid> & outGameGrids) const
{
	size_t maxRow = mGrid.numRows() - inBlock.grid().numRows();
	for (size_t rowIdx = 0; rowIdx <= maxRow; ++rowIdx)
	{
		if (!checkPositionValid(inBlock, rowIdx, inColIdx))
		{
			if (rowIdx > 0)
			{
				// we collided => solidify on position higher
				GameGrid gg(*this);
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
				GameGrid gg(*this);
				gg.solidifyBlockPosition(inBlock, rowIdx, inColIdx);
				outGameGrids.insert(gg);
				return true;
			}
		}
	}
	assert (!"We should not come here");
	return false;
}


int GameGrid::calculateScore() const
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
				if (value != Block::MAX_BLOCK)
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


bool GameGrid::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
{
	const size_t maxRows = mGrid.numRows() - inBlock.grid().numRows();
	const size_t maxCols = mGrid.numColumns() - inBlock.grid().numColumns();
	for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
	{
		for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
		{
			if (inBlock.grid().get(r, c) != 0 && mGrid.get(inRowIdx + r, inColIdx + c) != Block::MAX_BLOCK)
			{
				return false;
			}
		}
	}
	return true;
}


void GameGrid::solidifyBlockPosition(const Block & inBlock, size_t inRowIdx, size_t inColIdx)
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