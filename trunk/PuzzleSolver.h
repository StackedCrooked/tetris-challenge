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

		const GameStateNode * currentNode() const { return mCurrentNode; }

	private:
		void generateFutureGameStates(const GameState & inGameState, const Block & inBlock, GameStateNode::Children & outGameGrids);

		void generateFutureGameStates(const GameState & inGameState, const Block & inBlock, size_t inColIdx, GameStateNode::Children & outGameGrids) const;

		GameStateNode mRootNode;
		GameStateNode * mCurrentNode;
		std::vector<BlockIdentifier> mBlocks;
		size_t mTreeDepth;
	};

} // namespace Tetris

#endif // PUZZLESOLVER_H
