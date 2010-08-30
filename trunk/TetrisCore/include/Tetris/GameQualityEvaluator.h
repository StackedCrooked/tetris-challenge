#ifndef GAMEQUALITYEVALUATOR_H_INCLUDED
#define GAMEQUALITYEVALUATOR_H_INCLUDED


#include "Utilities.h"
#include "TypedWrapper.h"


namespace Tetris
{
    class GameState;

    GENERATE_TYPESAFE_WRAPPER(int, GameHeightFactor)
    GENERATE_TYPESAFE_WRAPPER(int, LastBlockHeightFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumHolesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumLines)
    GENERATE_TYPESAFE_WRAPPER(int, NumSinglesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumDoublesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumTriplesFactor)
    GENERATE_TYPESAFE_WRAPPER(int, NumTetrisesFactor)

    class Evaluator
    {
    public:
        Evaluator(GameHeightFactor inGameHeightFactor,
                  LastBlockHeightFactor inLastBlockHeightFactor, 
                  NumHolesFactor inNumHolesFactor,
                  NumSinglesFactor inNumSinglesFactor,
                  NumDoublesFactor inNumDoublesFactor,
                  NumTriplesFactor inNumTriplesFactor,
                  NumTetrisesFactor inNumTetrisesFactor);

        virtual std::auto_ptr<Evaluator> clone() const = 0 {}

        virtual int evaluate(const GameState & inGameState) const;

    private:
        int mGameHeightFactor;
        int mLastBlockHeightFactor;
        int mNumHolesFactor;
        int mNumSinglesFactor;
        int mNumDoublesFactor;
        int mNumTriplesFactor;
        int mNumTetrisesFactor;
    };


    class Balanced : public Evaluator
    {
    public:
        Balanced();

        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, Balanced>(*this); }
    };


    class Perfectionistic : public Evaluator
    {
    public:
        Perfectionistic();

        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, Perfectionistic>(*this); }
    };


} // namespace Tetris


#endif // GAMEQUALITYEVALUATOR_H_INCLUDED
