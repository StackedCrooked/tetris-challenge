#ifndef TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED
#define TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED


#include "Utilities.h"
#include "TypedWrapper.h"


namespace Tetris
{

    class GameState;


    // Type-safe int wrappers to prevent mix-ups in arg lists.
    GENERATE_TYPESAFE_WRAPPER(int, GameHeightFactor)
    GENERATE_TYPESAFE_WRAPPER(int, LastBlockHeightFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumHolesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumLines)
    GENERATE_TYPESAFE_WRAPPER(int, NumSinglesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumDoublesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumTriplesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumTetrisesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, SearchDepth)
    GENERATE_TYPESAFE_WRAPPER(int, SearchWidth)


    class Evaluator
    {
    public:
        Evaluator(GameHeightFactor inGameHeightFactor,
                  LastBlockHeightFactor inLastBlockHeightFactor,
                  NumHolesFactor inNumHolesFactor,
                  NumSinglesFactor inNumSinglesFactor,
                  NumDoublesFactor inNumDoublesFactor,
                  NumTriplesFactor inNumTriplesFactor,
                  NumTetrisesFactor inNumTetrisesFactor,
                  SearchDepth inRecommendedSearchDepth,
                  SearchWidth inRecommendedSearchWidth);

        virtual std::auto_ptr<Evaluator> clone() const
        { return Create<Evaluator>(*this); }

        virtual int evaluate(const GameState & inGameState) const;

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
        { return CreatePoly<Evaluator, CustomEvaluator>(*this); }
    };


    class Balanced : public Evaluator
    {
    public:
        Balanced();

        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, Balanced>(*this); }
    };


    class Survival : public Evaluator
    {
    public:
        Survival();

        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, Survival>(*this); }
    };


    class MakeTetrises : public Evaluator
    {
    public:
        MakeTetrises();

        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, MakeTetrises>(*this); }
    };


    class Depressed : public Evaluator
    {
    public:
        Depressed();

        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, Depressed>(*this); }
    };


} // namespace Tetris


#endif // GAMEQUALITYEVALUATOR_H_INCLUDED
