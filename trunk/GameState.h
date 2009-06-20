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

		int calculateScore() const;

		bool hasTopHoles() const;

		bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

		GameState makeGameStateWithAddedBlock(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

		int numHoles() const;

	private:
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
