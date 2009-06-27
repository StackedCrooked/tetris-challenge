#include "PuzzleSolver.h"
#include "GameState.h"
#include <ctime>
#include <iostream>
#include <string>
#include <vector>


namespace Tetris
{


	void printState(const GenericGrid<char> & grid, std::ostream & out)
	{
		for (size_t rowIdx = 0; rowIdx != grid.numRows(); ++rowIdx)
		{
			for (size_t colIdx = 0; colIdx != grid.numColumns(); ++colIdx)
			{
				out << grid.get(rowIdx, colIdx);
			}
			out << std::endl;
		}
	}

	void printListOfMoves(const std::vector<std::string> & moves, std::ostream & out)
	{
		for (size_t idx = 0; idx != moves.size(); ++idx)
		{
			out << moves[idx] << std::endl;
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

	Tetris::GenericGrid<char> grid(15, 15, '.');
	puzzleSolver.getAsciiFormat(grid);
	Tetris::printState(grid, std::cout);

	std::vector<std::string> moves;
	puzzleSolver.getListOfMoves(moves);
	Tetris::printListOfMoves(moves, std::cout);
	
	
	std::cout << "Press ENTER to quit";
	getchar();
	return 0;
}
