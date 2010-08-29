#ifndef GAMEQUALITYEVALUATOR_H_INCLUDED
#define GAMEQUALITYEVALUATOR_H_INCLUDED


#include "Utilities.h"


namespace Tetris
{
    class GameState;

    class Evaluator
    {
    public:
        Evaluator();

        int evaluate(const GameState & inGameState) const;

        virtual std::auto_ptr<Evaluator> clone() const = 0 {}

    protected:
        virtual int evaluateImpl(const GameState & inGameState,
                                 int inGameHeight,
                                 int inLastBlockHeight,
                                 int inNumHoles,
                                 int inNumOccupiedUnderTop,
                                 float inDensity) const = 0;
    };


    class DefaultEvaluator : public Evaluator
    {
    public:
        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, DefaultEvaluator>(*this); }

    protected:

        virtual int evaluateImpl(const GameState & inGameState,
                                 int inGameHeight,
                                 int inLastBlockHeight,
                                 int inNumHoles,
                                 int inNumOccupiedUnderTop,
                                 float inDensity) const;
    };


    class MakeTetrises : public Evaluator
    {
    public:
        virtual std::auto_ptr<Evaluator> clone() const
        { return CreatePoly<Evaluator, MakeTetrises>(*this); }

    protected:
        virtual int evaluateImpl(const GameState & inGameState,
                                 int inGameHeight,
                                 int inLastBlockHeight,
                                 int inNumHoles,
                                 int inNumOccupiedUnderTop,
                                 float inDensity) const;
    };


} // namespace Tetris


#endif // GAMEQUALITYEVALUATOR_H_INCLUDED
