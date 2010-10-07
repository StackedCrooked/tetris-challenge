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
                         NumTetrisesFactor inNumTetrisesFactor,
                         SearchDepth inRecommendedSearchDepth,
                         SearchWidth inRecommendedSearchWidth) :
        mGameHeightFactor(inGameHeightFactor.get()),
        mLastBlockHeightFactor(inLastBlockHeightFactor.get()),
        mNumHolesFactor(inNumHolesFactor.get()),
        mNumSinglesFactor(inNumSinglesFactor.get()),
        mNumDoublesFactor(inNumDoublesFactor.get()),
        mNumTriplesFactor(inNumTriplesFactor.get()),
        mNumTetrisesFactor(inNumTetrisesFactor.get()),
        mRecommendedSearchDepth(inRecommendedSearchDepth.get()),
        mRecommendedSearchWidth(inRecommendedSearchWidth.get())
    {
    }


    int Evaluator::gameHeightFactor() const
    {
        return mGameHeightFactor;
    }


    int Evaluator::lastBlockHeightFactor() const
    {
        return mLastBlockHeightFactor;
    }


    int Evaluator::numHolesFactor() const
    {
        return mNumHolesFactor;
    }


    int Evaluator::numSinglesFactor() const
    {
        return mNumSinglesFactor;
    }


    int Evaluator::numDoublesFactor() const
    {
        return mNumDoublesFactor;
    }


    int Evaluator::numTriplesFactor() const
    {
        return mNumTriplesFactor;
    }


    int Evaluator::numTetrisesFactor() const
    {
        return mNumTetrisesFactor;
    }


    int Evaluator::recommendedSearchDepth() const
    {
        return mRecommendedSearchDepth;
    }


    int Evaluator::recommendedSearchWidth() const
    {
        return mRecommendedSearchWidth;
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


    CustomEvaluator::CustomEvaluator(GameHeightFactor inGameHeightFactor,
                                     LastBlockHeightFactor inLastBlockHeightFactor,
                                     NumHolesFactor inNumHolesFactor,
                                     NumSinglesFactor inNumSinglesFactor,
                                     NumDoublesFactor inNumDoublesFactor,
                                     NumTriplesFactor inNumTriplesFactor,
                                     NumTetrisesFactor inNumTetrisesFactor,
                                     SearchDepth inRecommendedSearchDepth,
                                     SearchWidth inRecommendedSearchWidth) :
        Evaluator(inGameHeightFactor,
                  inLastBlockHeightFactor,
                  inNumHolesFactor,
                  inNumSinglesFactor,
                  inNumDoublesFactor,
                  inNumTriplesFactor,
                  inNumTetrisesFactor,
                  inRecommendedSearchDepth,
                  inRecommendedSearchWidth)
    {
    }


    Balanced::Balanced() :
        Evaluator(GameHeightFactor(-2),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(-4),
                  NumSinglesFactor(1),
                  NumDoublesFactor(2),
                  NumTriplesFactor(4),
                  NumTetrisesFactor(8),
                  SearchDepth(8),
                  SearchWidth(4))
    {
    }


    Survival::Survival() :
        Evaluator(GameHeightFactor(-2),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(-2),
                  NumSinglesFactor(1),
                  NumDoublesFactor(2),
                  NumTriplesFactor(4),
                  NumTetrisesFactor(8),
                  SearchDepth(4),
                  SearchWidth(4))
    {
    }


    MakeTetrises::MakeTetrises() :
        Evaluator(GameHeightFactor(-2),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(-4),
                  NumSinglesFactor(-4),
                  NumDoublesFactor(-8),
                  NumTriplesFactor(-8),
                  NumTetrisesFactor(8),
                  SearchDepth(8),
                  SearchWidth(5))
    {
    }


    Depressed::Depressed() :
        Evaluator(GameHeightFactor(0),
                  LastBlockHeightFactor(0),
                  NumHolesFactor(0),
                  NumSinglesFactor(0),
                  NumDoublesFactor(0),
                  NumTriplesFactor(0),
                  NumTetrisesFactor(0),
                  SearchDepth(1),
                  SearchWidth(1))
    {
    }


    Shabby::Shabby() :
        Evaluator(GameHeightFactor(-1),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(0),
                  NumSinglesFactor(0),
                  NumDoublesFactor(0),
                  NumTriplesFactor(0),
                  NumTetrisesFactor(0),
                  SearchDepth(4),
                  SearchWidth(4))
    {
    }


} // namespace Tetris
