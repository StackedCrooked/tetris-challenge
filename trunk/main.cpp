#include "PuzzleSolver.h"
#include "GameState.h"
#include <iostream>
#include <ctime>


namespace Tetris
{
	void fillCharGrid(const GameStateNode * node, GenericGrid<char> & grid)
	{
		GameStateNode::Children::const_iterator it = node->children().begin(), end = node->children().end();
		for (; it != end; ++it)
		{
			const GameStateNode * childNode = it->get();
			if (!childNode->isDeadEnd())
			{
				char id = childNode->lastBlock().charId();
				size_t row, col;
				childNode->lastBlockPosition(row, col);
				for (size_t r = row; r != row + childNode->lastBlock().grid().numRows(); ++r)
				{
					for (size_t c = col; c != col + childNode->lastBlock().grid().numColumns(); ++c)
					{
						if (childNode->lastBlock().grid().get(r - row, c - col) != NO_BLOCK)
						{
							grid.set(r, c, id);
						}
					}
				}
				fillCharGrid(childNode, grid);
				return;
			}
		}
	}


	void printState(const GameStateNode * inRootNode)
	{
		GenericGrid<char> grid(15, 15, '.');
		const GameStateNode * node = inRootNode;
		fillCharGrid(node, grid);
		for (size_t rowIdx = 0; rowIdx != grid.numRows(); ++rowIdx)
		{
			for (size_t colIdx = 0; colIdx != grid.numColumns(); ++colIdx)
			{
				std::cout << grid.get(rowIdx, colIdx);
			}
			std::cout << std::endl;
		}
	}

}


int main()
{
	std::cout << "Running..." << std::endl;
	std::time_t start(time(0));
	Tetris::PuzzleSolver puzzleSolver;
	int count = 0;
	int maxDepth = 0;
	while (puzzleSolver.depth() != 56)
	{
		puzzleSolver.next();
		if (puzzleSolver.depth() > maxDepth)
		{
			std::cout << "Depth has reached: " << puzzleSolver.depth() << ". Count: " << count << std::endl;
			maxDepth = puzzleSolver.depth();
		}
		if (count % 10000 == 0)
		{
			std::cout << "Count: " << count << std::endl;
		}
		count++;
	}
	std::cout << "Duration: " << time(0) - start << " s" << std::endl;	

	printState(puzzleSolver.rootNode());
	std::cout << "Press ENTER to quit";
	getchar();
	return 0;
}
