#ifndef GAME_H
#define GAME_H

#include "Block.h"
#include "GenericGrid.h"
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

		bool isDeadEnd() const { return mDeadEnd; }

		void markAsDeadEnd() const { mDeadEnd = true; }

	private:
		Grid mGrid;

		mutable bool mDirty;
		mutable int mCachedScore;

		mutable bool mDeadEnd;
	};

		
	bool operator<(const GameState & lhs, const GameState & rhs);


} // namespace Tetris


#endif // GAME_H
