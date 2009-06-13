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
		GameState();

		const Grid & grid() const;

		Grid & grid();

		int calculateScore() const;

		bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

		GameState makeGameStateWithAddedBlock(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

	private:
		Grid mGrid;

		mutable bool mDirty;
		mutable int mCachedScore;
	};

		
	bool operator<(const GameState & lhs, const GameState & rhs);


} // namespace Tetris


#endif // GAME_H
