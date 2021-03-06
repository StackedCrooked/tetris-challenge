#include "Tetris/Config.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Futile/Threading.h"


using Futile::ScopedLock;


namespace Tetris {


Evaluator::Evaluator(const std::string& inName,
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


int Evaluator::evaluate(const GameState& inGameState) const
{
    const Grid& grid = inGameState.grid();
    std::size_t top = grid.rowCount();
    bool foundTop = false;
    int numHoles = 0;
    int numOccupiedUnderTop = 0;

    for (std::size_t rowIdx = 0; rowIdx != grid.rowCount(); ++rowIdx)
    {
        for (std::size_t colIdx = 0; colIdx != grid.columnCount(); ++colIdx)
        {
            const int& value = grid.get(rowIdx, colIdx);
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
    Super("Balanced",
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
    Super("Survival",
          GameHeightFactor(-2),
          LastBlockHeightFactor(-1),
          NumHolesFactor(-3),
          NumSinglesFactor(1),
          NumDoublesFactor(2),
          NumTriplesFactor(4),
          NumTetrisesFactor(8),
          SearchDepth(6),
          SearchWidth(4))
{
}


MakeTetrises::MakeTetrises() :
    Super("Make Tetrises",
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


int MakeTetrises::evaluate(const GameState& inGameState) const
{
    const Grid& grid = inGameState.grid();

    std::size_t c = grid.columnCount() - 1;
    if (grid.rowCount() >= 4)
    {
        std::size_t r = grid.rowCount() - 4;
        for (; r != grid.rowCount(); ++r)
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
              SearchWidth(4))
{
}


Confused::Confused() :
    Super("Depressed",
    GameHeightFactor(-2),
    LastBlockHeightFactor(-2),  // Height is not OK.
    NumHolesFactor(1),          // But holes are desirable!
    NumSinglesFactor(1),
    NumDoublesFactor(2),
    NumTriplesFactor(4),
    NumTetrisesFactor(64),
    SearchDepth(20),
    SearchWidth(4))
{
}


int Confused::evaluate(const GameState& inGameState) const
{
    int result = 0;

    auto current_height = inGameState.currentHeight();

    if (current_height > 4)
    {
        result -= current_height * current_height;
    }

    return Evaluator::evaluate(inGameState) + result;
}


} // namespace Tetris
