#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameState.h"


namespace Tetris
{

    Evaluator::Evaluator(GameHeightFactor inGameHeightFactor,
                         LastBlockHeightFactor inLastBlockHeightFactor, 
                         NumHolesFactor inNumHolesFactor,
                         NumSinglesFactor inNumSinglesFactor,
                         NumDoublesFactor inNumDoublesFactor,
                         NumTriplesFactor inNumTriplesFactor,
                         NumTetrisesFactor inNumTetrisesFactor) :
        mGameHeightFactor(inGameHeightFactor.get()),
        mLastBlockHeightFactor(inLastBlockHeightFactor.get()),
        mNumHolesFactor(inNumHolesFactor.get()),
        mNumSinglesFactor(inNumSinglesFactor.get()),
        mNumDoublesFactor(inNumDoublesFactor.get()),
        mNumTriplesFactor(inNumTriplesFactor.get()),
        mNumTetrisesFactor(inNumTetrisesFactor.get())
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


        return gameHeight * mGameHeightFactor + 
               lastBlockHeight * mLastBlockHeightFactor +
               numHoles * mNumHolesFactor +
               inGameState.stats().numSingles() * mNumSinglesFactor +
               inGameState.stats().numDoubles() * mNumDoublesFactor +
               inGameState.stats().numTriples() * mNumTriplesFactor +
               inGameState.stats().numTetrises() * mNumTetrisesFactor;
    }


    Balanced::Balanced() :
        Evaluator(GameHeightFactor(-2),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(-4),
                  NumSinglesFactor(1),
                  NumDoublesFactor(2),
                  NumTriplesFactor(4),
                  NumTetrisesFactor(8))
    {
    }


    Perfectionistic::Perfectionistic() :
        Evaluator(GameHeightFactor(-4),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(-16),
                  NumSinglesFactor(-4),
                  NumDoublesFactor(-4),
                  NumTriplesFactor(-4),
                  NumTetrisesFactor(8))
    {
    }


} // namespace Tetris
