#ifndef TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED
#define TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED


#include "Futile/AutoPtrSupport.h"
#include "Futile/Threading.h"
#include "Futile/TypedWrapper.h"
#include <memory>
#include <string>


namespace Tetris {


class GameState;


// Type-safe int wrappers to prevent mix-ups in arg lists.
Futile_TypedWrapper(GameHeightFactor, int);
Futile_TypedWrapper(LastBlockHeightFactor, int);
Futile_TypedWrapper(NumHolesFactor, int);
Futile_TypedWrapper(NumLines, int);
Futile_TypedWrapper(NumSinglesFactor, int);
Futile_TypedWrapper(NumDoublesFactor, int);
Futile_TypedWrapper(NumTriplesFactor, int);
Futile_TypedWrapper(NumTetrisesFactor, int);
Futile_TypedWrapper(SearchDepth, int);
Futile_TypedWrapper(SearchWidth, int);


class Evaluator
{
public:
    Evaluator(const std::string & inName,
              GameHeightFactor inGameHeightFactor,
              LastBlockHeightFactor inLastBlockHeightFactor,
              NumHolesFactor inNumHolesFactor,
              NumSinglesFactor inNumSinglesFactor,
              NumDoublesFactor inNumDoublesFactor,
              NumTriplesFactor inNumTriplesFactor,
              NumTetrisesFactor inNumTetrisesFactor,
              SearchDepth inRecommendedSearchDepth,
              SearchWidth inRecommendedSearchWidth);

    virtual std::auto_ptr<Evaluator> clone() const = 0;

    virtual int evaluate(const GameState & inGameState) const;

    // Return by value to prevent race conditions.
    std::string name() const;

    int gameHeightFactor() const;

    int lastBlockHeightFactor() const;

    int numHolesFactor() const;

    int numSinglesFactor() const;

    int numDoublesFactor() const;

    int numTriplesFactor() const;

    int numTetrisesFactor() const;

    int recommendedSearchDepth() const;

    int recommendedSearchWidth() const;

private:
    std::string mName;
    int mGameHeightFactor;
    int mLastBlockHeightFactor;
    int mNumHolesFactor;
    int mNumSinglesFactor;
    int mNumDoublesFactor;
    int mNumTriplesFactor;
    int mNumTetrisesFactor;
    int mRecommendedSearchDepth;
    int mRecommendedSearchWidth;
};


class CustomEvaluator : public Evaluator
{
public:
    CustomEvaluator(GameHeightFactor inGameHeightFactor,
                    LastBlockHeightFactor inLastBlockHeightFactor,
                    NumHolesFactor inNumHolesFactor,
                    NumSinglesFactor inNumSinglesFactor,
                    NumDoublesFactor inNumDoublesFactor,
                    NumTriplesFactor inNumTriplesFactor,
                    NumTetrisesFactor inNumTetrisesFactor,
                    SearchDepth inRecommendedSearchDepth,
                    SearchWidth inRecommendedSearchWidth);

    virtual std::auto_ptr<Evaluator> clone() const
    { return Futile::CreatePoly<Evaluator, CustomEvaluator>(*this); }
};


class Balanced : public Evaluator
{
public:
    Balanced();

    virtual std::auto_ptr<Evaluator> clone() const
    { return Futile::CreatePoly<Evaluator, Balanced>(*this); }
};


class Survival : public Evaluator
{
public:
    Survival();

    virtual std::auto_ptr<Evaluator> clone() const
    { return Futile::CreatePoly<Evaluator, Survival>(*this); }
};


class MakeTetrises : public Evaluator
{
public:
    MakeTetrises();

    virtual std::auto_ptr<Evaluator> clone() const
    { return Futile::CreatePoly<Evaluator, MakeTetrises>(*this); }

    virtual int evaluate(const GameState & inGameState) const;
};


class Multiplayer : public Evaluator
{
public:
    Multiplayer();

    virtual std::auto_ptr<Evaluator> clone() const
    { return Futile::CreatePoly<Evaluator, Multiplayer>(*this); }
};


class Depressed : public Evaluator
{
public:
    Depressed();

    virtual std::auto_ptr<Evaluator> clone() const
    { return Futile::CreatePoly<Evaluator, Depressed>(*this); }
};


} // namespace Tetris


#endif // GAMEQUALITYEVALUATOR_H_INCLUDED
