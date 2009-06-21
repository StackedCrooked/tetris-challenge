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

		void populateNode(GameStateNode * inNode, const std::vector<Block> & inBlockTypes) const;

		const GameStateNode * currentNode() const;

		const GameStateNode * rootNode() const;

		const std::vector<Block> & blocks() const { return mBlocks; }

		size_t depth() const { return mNodes.size(); }

		void getAsciiFormat(GenericGrid<char> & grid) const;

	private:
		void tryNextBranch();

		int depthOfOffspring(const GameStateNode * inGameStateNode, std::list<GameStateNode*> & outNodes) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlockId, GameStateNode::Children & outGameGrids) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, size_t inColIdx, GameStateNode::Children & outGameGrids) const;

		void getAsciiFormat(const GameStateNode * inNode, GenericGrid<char> & grid) const;

		GameStateNode mRootNode;
		std::vector<Block> mBlocks;
		typedef std::list<GameStateNode::Children::iterator> Nodes;
		Nodes mNodes;
	};

} // namespace Tetris

#endif // PUZZLESOLVER_H
