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
FUTILE_BOX_TYPE(GameHeightFactor, int);
FUTILE_BOX_TYPE(LastBlockHeightFactor, int);
FUTILE_BOX_TYPE(NumHolesFactor, int);
FUTILE_BOX_TYPE(NumLines, int);
FUTILE_BOX_TYPE(NumSinglesFactor, int);
FUTILE_BOX_TYPE(NumDoublesFactor, int);
FUTILE_BOX_TYPE(NumTriplesFactor, int);
FUTILE_BOX_TYPE(NumTetrisesFactor, int);
FUTILE_BOX_TYPE(SearchDepth, int);
FUTILE_BOX_TYPE(SearchWidth, int);


class Evaluator
{
public:
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

protected:
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
};


template<class SubType>
class ConcreteEvaluator : public Evaluator
{
public:
    static SubType & Instance()
    {
        static SubType fInstance;
        return fInstance;
    }

protected:
    ConcreteEvaluator(const std::string & inName,
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
};


class Balanced : public ConcreteEvaluator<Balanced>
{
protected:
    typedef ConcreteEvaluator<Balanced> Super;
    friend class ConcreteEvaluator<Balanced>;

    Balanced();
};


class Survival : public ConcreteEvaluator<Survival>
{
protected:
    typedef ConcreteEvaluator<Survival> Super;
    friend class ConcreteEvaluator<Survival>;

    Survival();
};


class MakeTetrises : public ConcreteEvaluator<MakeTetrises>
{
public:
    virtual int evaluate(const GameState & inGameState) const;

protected:
    typedef ConcreteEvaluator<MakeTetrises> Super;
    friend class ConcreteEvaluator<MakeTetrises>;

    MakeTetrises();
};


class Multiplayer : public ConcreteEvaluator<Multiplayer>
{
protected:
    typedef ConcreteEvaluator<Multiplayer> Super;
    friend class ConcreteEvaluator<Multiplayer>;

    Multiplayer();
};


class Depressed : public ConcreteEvaluator<Depressed>
{
protected:
    typedef ConcreteEvaluator<Depressed> Super;
    friend class ConcreteEvaluator<Depressed>;

    Depressed();
};


} // namespace Tetris


#endif // GAMEQUALITYEVALUATOR_H_INCLUDED
