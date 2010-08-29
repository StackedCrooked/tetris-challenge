#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameState.h"


namespace Tetris
{

    Evaluator::Evaluator()
    {
    }


    int Evaluator::evaluate(const GameState & inGameState) const
    {
        const Grid & grid = inGameState.grid(); 
        size_t top = grid.numRows();    
        bool foundTop = false;   
        int numHoles = 0;
        int numOccupiedUnderTop = 0;

        for (size_t rowIdx = 0; rowIdx != grid.numRows(); ++rowIdx)
        {
            for (size_t colIdx = 0; colIdx != grid.numColumns(); ++colIdx)
            {
                const int & value = grid.get(rowIdx, colIdx);
                if (value != BlockType_Nil)
                {
                    if (!foundTop)
                    {
                        top = rowIdx;
                        foundTop = true;
                    }

                    if (foundTop)
                    {
                        numOccupiedUnderTop++;
                    }
                }
                else
                {
                    // check for holes
                    if (foundTop && rowIdx > 0)
                    {
                        if (grid.get(rowIdx - 1, colIdx) != BlockType_Nil)
                        {
                            numHoles++;
                        }
                    }
                }

            }
        }
        int gameHeight = grid.numRows() - top;
        int lastBlockHeight = grid.numRows() - inGameState.originalBlock().row();
        float density = static_cast<float>(numOccupiedUnderTop)/static_cast<float>(gameHeight * grid.numColumns());
        return evaluateImpl(inGameState, gameHeight, lastBlockHeight, numHoles, numOccupiedUnderTop, density);
    }


    int DefaultEvaluator::evaluateImpl(const GameState & inGameState,
                                                  int inGameHeight,
                                                  int inLastBlockHeight,
                                                  int inNumHoles,
                                                  int inNumOccupiedUnderTop,
                                                  float inDensity) const
    {
        return 0
               - 2 * inGameHeight
               - 1 * inLastBlockHeight
               - 4 * inNumHoles
               + 1 * inGameState.stats().numSingles()
               + 2 * inGameState.stats().numDoubles()
               + 4 * inGameState.stats().numTriples()
               + 8 * inGameState.stats().numTetrises();
    }


    int MakeTetrises::evaluateImpl(const GameState & inGameState,
                                                  int inGameHeight,
                                                  int inLastBlockHeight,
                                                  int inNumHoles,
                                                  int inNumOccupiedUnderTop,
                                                  float inDensity) const
    {
        return 0
               - 4 * inGameHeight
               - 1 * inLastBlockHeight
               - 16 * inNumHoles
               - 8 * inGameState.stats().numSingles()
               - 8 * inGameState.stats().numDoubles()
               - 8 * inGameState.stats().numTriples()
               + 8 * inGameState.stats().numTetrises();
    }

} // namespace Tetris
