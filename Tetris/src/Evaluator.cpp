#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include <cmath>


namespace Tetris {


using namespace Futile;


Evaluator::Evaluator(const std::string & inName,
                     GameHeightFactor inGameHeightFactor,
                     LastBlockHeightFactor inLastBlockHeightFactor,
                     NumHolesFactor inNumHolesFactor,
                     NumSinglesFactor inNumSinglesFactor,
                     NumDoublesFactor inNumDoublesFactor,
                     NumTriplesFactor inNumTriplesFactor,
                     NumTetrisesFactor inNumTetrisesFactor,
                     SearchDepth inRecommendedSearchDepth,
                     SearchWidth inRecommendedSearchWidth,
                     MoveDownBehavior inMoveDownBehavior) :
    mName(inName),
    mGameHeightFactor(inGameHeightFactor),
    mLastBlockHeightFactor(inLastBlockHeightFactor),
    mNumHolesFactor(inNumHolesFactor),
    mNumSinglesFactor(inNumSinglesFactor),
    mNumDoublesFactor(inNumDoublesFactor),
    mNumTriplesFactor(inNumTriplesFactor),
    mNumTetrisesFactor(inNumTetrisesFactor),
    mRecommendedSearchDepth(inRecommendedSearchDepth),
    mRecommendedSearchWidth(inRecommendedSearchWidth),
    mMoveDownBehavior(inMoveDownBehavior)
{
}


std::string Evaluator::name() const
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
    std::size_t top = grid.rowCount();
    bool foundTop = false;
    int numHoles = 0;
    int numOccupiedUnderTop = 0;

    for (std::size_t rowIdx = 0; rowIdx != grid.rowCount(); ++rowIdx)
    {
        for (std::size_t colIdx = 0; colIdx != grid.columnCount(); ++colIdx)
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
    int gameHeight = grid.rowCount() - top;
    int lastBlockHeight = grid.rowCount() - inGameState.originalBlock().row();


    return gameHeight * mGameHeightFactor +
           lastBlockHeight * mLastBlockHeightFactor +
           numHoles * mNumHolesFactor +
           inGameState.numSingles() * mNumSinglesFactor +
           inGameState.numDoubles() * mNumDoublesFactor +
           inGameState.numTriples() * mNumTriplesFactor +
           inGameState.numTetrises() * mNumTetrisesFactor;
}


CustomEvaluator::CustomEvaluator(GameHeightFactor inGameHeightFactor,
                                 LastBlockHeightFactor inLastBlockHeightFactor,
                                 NumHolesFactor inNumHolesFactor,
                                 NumSinglesFactor inNumSinglesFactor,
                                 NumDoublesFactor inNumDoublesFactor,
                                 NumTriplesFactor inNumTriplesFactor,
                                 NumTetrisesFactor inNumTetrisesFactor,
                                 SearchDepth inRecommendedSearchDepth,
                                 SearchWidth inRecommendedSearchWidth,
                                 MoveDownBehavior inMoveDownBehavior) :
    Evaluator("Custom",
              inGameHeightFactor,
              inLastBlockHeightFactor,
              inNumHolesFactor,
              inNumSinglesFactor,
              inNumDoublesFactor,
              inNumTriplesFactor,
              inNumTetrisesFactor,
              inRecommendedSearchDepth,
              inRecommendedSearchWidth,
              inMoveDownBehavior)
{
}


Balanced::Balanced() :
    Super("Balanced",
          GameHeightFactor(-2),
          LastBlockHeightFactor(-1),
          NumHolesFactor(-4),
          NumSinglesFactor(1),
          NumDoublesFactor(2),
          NumTriplesFactor(4),
          NumTetrisesFactor(8),
          SearchDepth(6),
          SearchWidth(6),
          MoveDownBehavior_None)
{
}


Survival::Survival() :
    Super("Survival",
          GameHeightFactor(-2),
          LastBlockHeightFactor(-1),
          NumHolesFactor(-3),
          NumSinglesFactor(1),
          NumDoublesFactor(2),
          NumTriplesFactor(4),
          NumTetrisesFactor(8),
          SearchDepth(4),
          SearchWidth(4),
          MoveDownBehavior_MoveDown)
{
}


MakeTetrises::MakeTetrises() :
    Super("Make Tetrises",
          GameHeightFactor(-2),
          LastBlockHeightFactor(-1),
          NumHolesFactor(-4),
          NumSinglesFactor(-4),
          NumDoublesFactor(-8),
          NumTriplesFactor(-32),
          NumTetrisesFactor(1),
          SearchDepth(8),
          SearchWidth(4),
          MoveDownBehavior_MoveDown)
{
}


int MakeTetrises::evaluate(const GameState & inGameState) const
{
    const Grid & grid = inGameState.grid();

    double result = std::pow(inGameState.firstOccupiedRow(), 0.5);


    int c = grid.columnCount() - 1;
    if (grid.rowCount() >= 4)
    {
        int r = std::min(0, inGameState.firstOccupiedRow() - 4);
        for (; r < int(grid.rowCount()); ++r)
        {
            Assert(r > 0 && r < int(grid.rowCount()));
            Assert(c > 0 && c < int(grid.columnCount()));
            if (grid.get(r, c))
            {
                // Penalty for occupying the last column,
                // which is reserved for making tetrises.
                return int(0.5 + result + Evaluator::evaluate(inGameState) - 4);
            }
        }
    }
    return int(0.5 + result + Evaluator::evaluate(inGameState));
}


Multiplayer::Multiplayer() :
    Super("Multiplayer",
          GameHeightFactor(-2),
          LastBlockHeightFactor(-1),
          NumHolesFactor(-4),
          NumSinglesFactor(-4),
          NumDoublesFactor(-4),
          NumTriplesFactor(8),
          NumTetrisesFactor(16),
          SearchDepth(8),
          SearchWidth(4),
          MoveDownBehavior_MoveDown)
{
}


Depressed::Depressed() :
    Super("Depressed",
          GameHeightFactor(0),
          LastBlockHeightFactor(0),
          NumHolesFactor(0),
          NumSinglesFactor(0),
          NumDoublesFactor(0),
          NumTriplesFactor(0),
          NumTetrisesFactor(0),
          SearchDepth(1),
          SearchWidth(1),
          MoveDownBehavior_DropDown)
{
}


} // namespace Tetris
