#ifndef TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED
#define TETRIS_GAMEQUALITYEVALUATOR_H_INCLUDED


#include "Futile/AutoPtrSupport.h"
#include "Futile/Singleton.h"
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
    virtual int evaluate(const GameState& inGameState) const;

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

protected:
    virtual ~Evaluator() {}

    Evaluator(const std::string& inName,
              GameHeightFactor inGameHeightFactor,
              LastBlockHeightFactor inLastBlockHeightFactor,
              NumHolesFactor inNumHolesFactor,
              NumSinglesFactor inNumSinglesFactor,
              NumDoublesFactor inNumDoublesFactor,
              NumTriplesFactor inNumTriplesFactor,
              NumTetrisesFactor inNumTetrisesFactor,
              SearchDepth inRecommendedSearchDepth,
              SearchWidth inRecommendedSearchWidth);

private:
    Evaluator(const Evaluator &);
    Evaluator& operator=(const Evaluator&);

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
protected:
    CustomEvaluator(GameHeightFactor inGameHeightFactor,
                    LastBlockHeightFactor inLastBlockHeightFactor,
                    NumHolesFactor inNumHolesFactor,
                    NumSinglesFactor inNumSinglesFactor,
                    NumDoublesFactor inNumDoublesFactor,
                    NumTriplesFactor inNumTriplesFactor,
                    NumTetrisesFactor inNumTetrisesFactor,
                    SearchDepth inRecommendedSearchDepth,
                    SearchWidth inRecommendedSearchWidth);

    virtual ~CustomEvaluator() {}
};


template<class SubType>
class ConcreteEvaluator : public Evaluator
{
public:
    static SubType& Instance()
    {
        static SubType fInstance;
        return fInstance;
    }

protected:
    ConcreteEvaluator(const std::string& inName,
                      GameHeightFactor inGameHeightFactor,
                      LastBlockHeightFactor inLastBlockHeightFactor,
                      NumHolesFactor inNumHolesFactor,
                      NumSinglesFactor inNumSinglesFactor,
                      NumDoublesFactor inNumDoublesFactor,
                      NumTriplesFactor inNumTriplesFactor,
                      NumTetrisesFactor inNumTetrisesFactor,
                      SearchDepth inRecommendedSearchDepth,
                      SearchWidth inRecommendedSearchWidth) :
        Evaluator(inName,
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

    virtual ~ConcreteEvaluator() {}
};


class Balanced : public ConcreteEvaluator<Balanced>
{
protected:
    typedef ConcreteEvaluator<Balanced> Super;
    friend class ConcreteEvaluator<Balanced>;

    Balanced();
    virtual ~Balanced() {}
};


class Survival : public ConcreteEvaluator<Survival>
{
protected:
    typedef ConcreteEvaluator<Survival> Super;
    friend class ConcreteEvaluator<Survival>;

    Survival();
    virtual ~Survival() {}
};


class MakeTetrises : public ConcreteEvaluator<MakeTetrises>
{
public:
    virtual int evaluate(const GameState& inGameState) const;

protected:
    typedef ConcreteEvaluator<MakeTetrises> Super;
    friend class ConcreteEvaluator<MakeTetrises>;

    MakeTetrises();
    virtual ~MakeTetrises() {}
};


class Multiplayer : public ConcreteEvaluator<Multiplayer>
{
protected:
    typedef ConcreteEvaluator<Multiplayer> Super;
    friend class ConcreteEvaluator<Multiplayer>;

    Multiplayer();
    virtual ~Multiplayer() {}
};


class Confused : public ConcreteEvaluator<Confused>
{
protected:
    typedef ConcreteEvaluator<Confused> Super;
    friend class ConcreteEvaluator<Confused>;

    Confused();
    virtual ~Confused() {}

private:
    int evaluate(const GameState& inGameState) const final;
};


} // namespace Tetris


#endif // GAMEQUALITYEVALUATOR_H_INCLUDED
