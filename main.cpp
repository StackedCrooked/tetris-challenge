#include "PuzzleSolver.h"
#include "GameState.h"
#include <iostream>
#include <ctime>


namespace Tetris
{


	void printState(const GenericGrid<char> & grid)
	{
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

	Tetris::GenericGrid<char> grid(15, 15, '.');
	puzzleSolver.getAsciiFormat(grid);
	printState(grid);
	
	
	std::cout << "Press ENTER to quit";
	getchar();
	return 0;
}
