#ifndef GAMEQUALITYEVALUATOR_H_INCLUDED
#define GAMEQUALITYEVALUATOR_H_INCLUDED


namespace Tetris
{
    class GameState;

    class GameQualityEvaluator
    {
    public:
        GameQualityEvaluator();

        int evaluate(const GameState & inGameState);

    protected:
        virtual int evaluateImpl(const GameState & inGameState,
                                 int inGameHeight,
                                 int inLastBlockHeight,
                                 int inNumHoles,
                                 int inNumOccupiedUnderTop,
                                 float inDensity) const = 0;
    };


    class DefaultGameQualityEvaluator : public GameQualityEvaluator
    {
    protected:
        virtual int evaluateImpl(const GameState & inGameState,
                                 int inGameHeight,
                                 int inLastBlockHeight,
                                 int inNumHoles,
                                 int inNumOccupiedUnderTop,
                                 float inDensity) const;
    };


    class MakeTetrises : public GameQualityEvaluator
    {
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
