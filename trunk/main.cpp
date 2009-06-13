#include "GenericGrid.h"
#include "Parser.h"
#include "Visualizer.h"
#include "PuzzleSolver.h"
#include <set>


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Tetris::PuzzleSolver puzzleSolver;
	while (puzzleSolver.next())
	{

	}
	return 0;
}
