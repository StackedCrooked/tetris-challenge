#ifndef GAMESTATE_H
#define GAMESTATE_H

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

		// Calculates the quality of the playing field.
		int calculateScore() const;

		// When most blocks have been dropped in the field sometimes
		// the remaining openings don't fit a tetris block anymore
		// (for example an opening consisting of 2 or 3 holes). This
		// makes it impossible to finish the puzzle, and thus is an
		// indicator that the current game state is a dead end.
		bool hasTopHoles() const;

		// Checks if a block can be placed at a given location without
		// overlapping with previously placed blocks.
		bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

		// Creates a copy of this gamestate with an added block at the given location.
		GameState makeGameStateWithAddedBlock(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

		// Returns the number of holes. A hole is an empty square
		// surrounded by non-empty squares. The puzzle only allows
		// 1 hole, otherwise it won't fit all blocks. So we can use
		// this information to indicate the current game state as a
		// dead end.
		int numHoles() const;

	private:
		// For a given square, counts the number of neighor squares
		// that have the same value. This is a helper method for
		// the method 'hasTopHoles'.
		int countNeighbors(size_t inRowIdx, size_t inColIdx) const;

		Grid mGrid;

		// check for top holes
		static GenericGrid<int> sHelperGrid;
		static int sMarker;

		mutable bool mDirty;
		mutable int mCachedScore;
		mutable int mNumHoles;
	};

		
	bool operator<(const GameState & lhs, const GameState & rhs);


} // namespace Tetris


#endif // GAMESTATE_H
