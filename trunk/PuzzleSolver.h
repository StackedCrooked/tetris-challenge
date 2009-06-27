#ifndef PUZZLESOLVER_H
#define PUZZLESOLVER_H

#include "GameStateNode.h"
#include <list>
#include <vector>

namespace Tetris
{
	// Programming Challenge 25a - Like Playing Tetris?
	// http://cplus.about.com/b/2009/06/06/programming-challenge-25a-like-playing-tetris.htm

	// The algorithm for solving the puzzle goes as follows:
	// - Generate a list of blocks by parsing the inputs file.
	// - Create a node called the root node. This node contains an empty gamestate.
	// - The root node marked as current node.
	// - Create a list of child nodes for the current node (by dropping the first block).
	//   This list is sorted by score (meaning quality of the playing field).
	//   The ordering is descending so that the first child represent the best game state,
	//   the second child the second best gamestate, and so on.
	// - The first child is now selected as current node and we repeat above procedure with next block.
	// - When we reach the situation where a block can no longer be added to the current
	//   child (because we can't find a valid location to drop it) we mark this child as
	//   a dead end and try the next child.
	// - When all children of a node have been marked as a dead end we mark the current
	//   node as a dead end, move one position up in the tree, and continue with the next
	//   child that is not yet marked as a dead end.
	// - This repeated until the solution is reached (all blocks have been added).
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

		// Generates the final ASCII formatted grid for the output file.
		void getAsciiFormat(GenericGrid<char> & grid) const;

		// Generates the list of block positions for the output file.
		void getListOfMoves(std::vector<std::string> & list) const;

	private:
		void tryNextBranch();

		int depthOfOffspring(const GameStateNode * inGameStateNode, std::list<GameStateNode*> & outNodes) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlockId, GameStateNode::Children & outGameGrids) const;

		void generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, size_t inColIdx, GameStateNode::Children & outGameGrids) const;

		void getAsciiFormat(const GameStateNode * inNode, GenericGrid<char> & grid) const;

		void getListOfMoves(const GameStateNode * inNode, std::vector<std::string> & list) const;

		GameStateNode mRootNode;
		std::vector<Block> mBlocks;
		typedef std::list<GameStateNode::Children::iterator> Nodes;
		Nodes mNodes;
	};

} // namespace Tetris

#endif // PUZZLESOLVER_H
