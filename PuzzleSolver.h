#ifndef PUZZLESOLVER_H
#define PUZZLESOLVER_H

#include "Block.h"
#include "GameState.h"
#include "GameStateNode.h"
#include <vector>

namespace Tetris
{

	class PuzzleSolver
	{
	public:
		PuzzleSolver();

		bool next();

		// returns true when the Block was dropped successfully
		// returns false when no suitable location was found within the bounds of the grid
		void generateFutureGameStates(const GameState & inGameState, const Block & inBlock, std::set<GameState> & outGameGrids);

		void generateFutureGameStates(const GameState & inGameState, const Block & inBlock, size_t inColIdx, std::set<GameState> & outGameGrids) const;

	private:
		GameStateNode mRootNode;
		std::vector<BlockIdentifier> mBlocks;
		size_t mTreeDepth;
	};

} // namespace Tetris

#endif // PUZZLESOLVER_H
