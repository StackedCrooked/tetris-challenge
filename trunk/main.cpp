#include "Visualizer.h"
#include "PuzzleSolver.h"
#include <set>


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Tetris::PuzzleSolver puzzleSolver;
	Tetris::Visualizer v(&puzzleSolver);
	v.show();
	return 0;
}
