#ifndef PUZZLESOLVER_H
#define PUZZLESOLVER_H

#include "Block.h"
#include "GameState.h"
#include "GameStateNode.h"
#include <list>
#include <vector>

namespace Tetris
{

	class PuzzleSolver
	{
	public:
		PuzzleSolver();

		bool next();

		void populateNode(GameStateNode * inNode, const std::vector<BlockType> & inBlockTypes) const;

		const GameStateNode * currentNode() const;

		const std::vector<BlockIdentifier> & blocks() const { return mBlocks; }

		size_t depth() const { return mNodes.size(); }

	private:
		void tryNextBranch();

		int depthOfOffspring(const GameStateNode * inGameStateNode, std::list<GameStateNode*> & outNodes) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, BlockType inBlockType, GameStateNode::Children & outGameGrids) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, size_t inColIdx, GameStateNode::Children & outGameGrids) const;

		GameStateNode mRootNode;
		std::vector<BlockIdentifier> mBlocks;
		typedef std::list<GameStateNode::Children::iterator> Nodes;
		Nodes mNodes;
		size_t mMinNodes;
	};

} // namespace Tetris

#endif // PUZZLESOLVER_H
