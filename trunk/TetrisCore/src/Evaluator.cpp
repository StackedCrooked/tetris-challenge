#include "Tetris/Config.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"


namespace Tetris
{

    Evaluator::Evaluator(const std::string & inName,
                         GameHeightFactor inGameHeightFactor,
                         LastBlockHeightFactor inLastBlockHeightFactor,
                         NumHolesFactor inNumHolesFactor,
                         NumSinglesFactor inNumSinglesFactor,
                         NumDoublesFactor inNumDoublesFactor,
                         NumTriplesFactor inNumTriplesFactor,
                         NumTetrisesFactor inNumTetrisesFactor,
                         SearchDepth inRecommendedSearchDepth,
                         SearchWidth inRecommendedSearchWidth) :
        mName(inName),
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


    const std::string & Evaluator::name() const
    {
        return mName;
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
        Evaluator("Custom",
                  inGameHeightFactor,
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
        Evaluator("Balanced",
                  GameHeightFactor(-2),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(-4),
                  NumSinglesFactor(1),
                  NumDoublesFactor(2),
                  NumTriplesFactor(4),
                  NumTetrisesFactor(8),
                  SearchDepth(6),
                  SearchWidth(6))
    {
    }


    Survival::Survival() :
        Evaluator("Survival",
                  GameHeightFactor(-2),
                  LastBlockHeightFactor(-6),
                  NumHolesFactor(-3),
                  NumSinglesFactor(1),
                  NumDoublesFactor(2),
                  NumTriplesFactor(4),
                  NumTetrisesFactor(8),
                  SearchDepth(5),
                  SearchWidth(5))
    {
    }


    MakeTetrises::MakeTetrises() :
        Evaluator("Make Tetrises",
                  GameHeightFactor(-2),
                  LastBlockHeightFactor(-1),
                  NumHolesFactor(-4),
                  NumSinglesFactor(-4),
                  NumDoublesFactor(-8),
                  NumTriplesFactor(-8),
                  NumTetrisesFactor(16),
                  SearchDepth(8),
                  SearchWidth(5))
    {
    }


    int MakeTetrises::evaluate(const GameState & inGameState) const
    {
        const Grid & grid = inGameState.grid();
    
        size_t c = grid.numColumns() - 1;
        if (grid.numRows() >= 4)
        {
            size_t r = grid.numRows() - 4;
            for (; r != grid.numRows(); ++r)
            {
                if (grid.get(r, c))
                {
                    // Penalty for occupying the last column,
                    // which is reserved for making tetrises.
                    return Evaluator::evaluate(inGameState) - 4;
                }
            }
        }
        return Evaluator::evaluate(inGameState);
    }


    Depressed::Depressed() :
        Evaluator("Depressed",
                  GameHeightFactor(0),
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

} // namespace Tetris
