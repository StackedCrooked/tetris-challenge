#ifndef GAME_H
#define GAME_H

#include "Block.h"
#include "GenericGrid.h"
#include <boost/signal.hpp>
#include <set>
#include <vector>

class GameGrid
{
public:
	//mutable boost::signal<void(const Block &, size_t, size_t)> BlockMoved;

	GameGrid();

	const GenericGrid<int> & grid() const;

	// returns true when the Block was dropped successfully
	// returns false when no suitable location was found within the bounds of the grid
	bool addBlock(const Block & inBlock, std::set<GameGrid> & outGameGrids);

	bool addBlock(const Block & inBlock, size_t inColIdx, std::set<GameGrid> & outGameGrids) const;

	int calculateScore() const;

private:
	bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

	void solidifyBlockPosition(const Block & inBlock, size_t inRowIdx, size_t inColIdx);

	GenericGrid<int> mGrid;

	mutable bool mDirty;
	mutable int mCachedScore;
};

	
bool operator<(const GameGrid & lhs, const GameGrid & rhs);


#endif // GAME_H
