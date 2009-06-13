#ifndef GAME_H
#define GAME_H

#include "Block.h"
#include "GenericGrid.h"
#include <boost/signal.hpp>
#include <set>
#include <vector>


namespace Tetris
{

	class GameState
	{
	public:
		//mutable boost::signal<void(const Block &, size_t, size_t)> BlockMoved;

		GameState();

		const GenericGrid<int> & grid() const;

		// returns true when the Block was dropped successfully
		// returns false when no suitable location was found within the bounds of the grid
		void generateFutureGameStates(const Block & inBlock, std::set<GameState> & outGameGrids);

		void generateFutureGameStates(const Block & inBlock, size_t inColIdx, std::set<GameState> & outGameGrids) const;

		int calculateScore() const;

	private:
		bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

		void solidifyBlockPosition(const Block & inBlock, size_t inRowIdx, size_t inColIdx);

		GenericGrid<int> mGrid;

		mutable bool mDirty;
		mutable int mCachedScore;
	};

		
	bool operator<(const GameState & lhs, const GameState & rhs);


} // namespace Tetris


#endif // GAME_H
