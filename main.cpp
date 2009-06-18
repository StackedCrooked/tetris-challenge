#include "PuzzleSolver.h"
#include <iostream>
#include <ctime>


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
		count++;
	}
	std::cout << "Count: " << count << ". Duration: " << time(0) - start << " s" << std::endl;	
	std::cout << "Press ENTER to quit";
	getchar();
	return 0;
}
