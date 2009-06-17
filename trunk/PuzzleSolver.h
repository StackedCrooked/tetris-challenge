#ifndef PUZZLESOLVER_H
#define PUZZLESOLVER_H

#include "GameStateNode.h"
#include <list>
#include <vector>

namespace Tetris
{
	// Programming Challenge 25a - Like Playing Tetris?
	// http://cplus.about.com/b/2009/06/06/programming-challenge-25a-like-playing-tetris.htm

	class PuzzleSolver
	{
	public:
		PuzzleSolver();

		bool next();

		void populateNode(GameStateNode * inNode, const std::vector<BlockIdentifier> & inBlockTypes) const;

		const GameStateNode * currentNode() const;

		const std::vector<BlockIdentifier> & blocks() const { return mBlocks; }

		size_t depth() const { return mNodes.size(); }

	private:
		void tryNextBranch();

		int depthOfOffspring(const GameStateNode * inGameStateNode, std::list<GameStateNode*> & outNodes) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, const BlockIdentifier & inBlockId, GameStateNode::Children & outGameGrids) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, size_t inColIdx, GameStateNode::Children & outGameGrids) const;

		GameStateNode mRootNode;
		std::vector<BlockIdentifier> mBlocks;
		typedef std::list<GameStateNode::Children::iterator> Nodes;
		Nodes mNodes;
		size_t mMinNodes;
	};

} // namespace Tetris

#endif // PUZZLESOLVER_H
