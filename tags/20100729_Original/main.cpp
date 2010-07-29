#include "CStopWatch.h"
#include "GameState.h"
#include "PuzzleSolver.h"
#include <ctime>
#include <fstream>
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
                if (colIdx != 0)
                {
                    out << " ";
                }
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

    double getTiming(size_t inNumberOfIterations)
    {
        CStopWatch stopWatch;
        stopWatch.startTimer();
        for (size_t idx = 0; idx != inNumberOfIterations; ++idx)
        {
            Tetris::PuzzleSolver puzzleSolver;
            while (puzzleSolver.depth() != 56)
            {
                puzzleSolver.next();
            }
        }
        stopWatch.stopTimer();
        return stopWatch.getElapsedTime() / inNumberOfIterations;
    }
}




int main()
{
    // Uncomment this to measure timings
    //std::cout << "Duration: " << Tetris::getTiming(10) << "s." << std::endl;


    Tetris::PuzzleSolver puzzleSolver;
    while (puzzleSolver.depth() != 56)
    {
        puzzleSolver.next();
    }

    std::ofstream outFile("output.txt");
    Tetris::GenericGrid<char> grid(15, 15, '.');
    puzzleSolver.getAsciiFormat(grid);
    Tetris::printState(grid, outFile);

    std::vector<std::string> moves;
    puzzleSolver.getListOfMoves(moves);
    Tetris::printListOfMoves(moves, outFile);


    std::cout << "Press ENTER to quit";
    getchar();
    return 0;
}
