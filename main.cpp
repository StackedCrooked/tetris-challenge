#include "CStopWatch.h"
#include "GameState.h"
#include "PuzzleSolver.h"
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
	CStopWatch stopWatch;
	stopWatch.startTimer();
	Tetris::PuzzleSolver puzzleSolver;
	int count = 0;
	int maxDepth = 0;
	while (puzzleSolver.depth() != 56)
	{
		puzzleSolver.next();
		if (puzzleSolver.depth() > maxDepth)
		{
			maxDepth = puzzleSolver.depth();
		}
		count++;
	}

	Tetris::GenericGrid<char> grid(15, 15, '.');
	puzzleSolver.getAsciiFormat(grid);
	Tetris::printState(grid, std::cout);

	std::vector<std::string> moves;
	puzzleSolver.getListOfMoves(moves);
	Tetris::printListOfMoves(moves, std::cout);
	
	
	stopWatch.stopTimer();
	std::cout << "Duration: " << stopWatch.getElapsedTime() << "." << std::endl;
	std::cout << "Press ENTER to quit";
	getchar();
	return 0;
}
