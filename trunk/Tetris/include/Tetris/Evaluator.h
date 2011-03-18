#ifndef TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED
#define TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED


#include "Tetris/TypedWrapper.h"
#include "Tetris/AutoPtrSupport.h"
#include <boost/thread.hpp>
#include <memory>
#include <string>


namespace Tetris
{

    class GameState;


    // Type-safe int wrappers to prevent mix-ups in arg lists.
    Tetris_TypedWrapper(GameHeightFactor, int);
    Tetris_TypedWrapper(LastBlockHeightFactor, int);
    Tetris_TypedWrapper(NumHolesFactor, int);
    Tetris_TypedWrapper(NumLines, int);
    Tetris_TypedWrapper(NumSinglesFactor, int);
    Tetris_TypedWrapper(NumDoublesFactor, int);
    Tetris_TypedWrapper(NumTriplesFactor, int);
    Tetris_TypedWrapper(NumTetrisesFactor, int);
    Tetris_TypedWrapper(SearchDepth, int);
    Tetris_TypedWrapper(SearchWidth, int);


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

        virtual ~Evaluator();

        Evaluator(const Evaluator& rhs);

        Evaluator& operator=(const Evaluator& rhs);

        virtual std::auto_ptr<Evaluator> clone() const
        { return Create<Evaluator>(*this); }

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
        mutable boost::mutex mMutex;
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

        virtual int evaluate(const GameState & inGameState) const;
    };


    class Multiplayer : public Evaluator
    {
    public:
        Multiplayer();

        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, Multiplayer>(*this); }
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
