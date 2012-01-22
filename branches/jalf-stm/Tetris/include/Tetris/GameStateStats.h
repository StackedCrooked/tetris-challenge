#ifndef TETRIS_GAMESTATESTATS_H
#define TETRIS_GAMESTATESTATS_H


namespace Tetris {


class GameState;


/**
 * GameStateStats contains game statistics.
 * Objects are immutible.
 */
class GameStateStats
{
public:
    GameStateStats() :
        mNumLines(0),
        mNumSingles(0),
        mNumDoubles(0),
        mNumTriples(0),
        mNumTetrises(0)
    {
    }

    GameStateStats(int inNumLines,
                   int inNumSingles,
                   int inNumDoubles,
                   int inNumTriples,
                   int inNumTetrises) :
        mNumLines(inNumLines),
        mNumSingles(inNumSingles),
        mNumDoubles(inNumDoubles),
        mNumTriples(inNumTriples),
        mNumTetrises(inNumTetrises)
    {
    }

    GameStateStats increment(unsigned inNumLines)
    {
        return GameStateStats(numLines()    + inNumLines,
                              numSingles()  + (inNumLines == 1 ? 1 : 0),
                              numDoubles()  + (inNumLines == 2 ? 1 : 0),
                              numTriples()  + (inNumLines == 3 ? 1 : 0),
                              numTetrises() + (inNumLines == 4 ? 1 : 0));
    }

    int numLines() const
    { return mNumLines; }

    int numSingles() const
    { return mNumSingles; }

    int numDoubles() const
    { return mNumDoubles; }

    int numTriples() const
    { return mNumTriples; }

    int numTetrises() const
    { return mNumTetrises; }

    int score() const
    {
        return   40 * mNumSingles +
                100 * mNumDoubles +
                300 * mNumTriples +
               1200 * mNumTetrises;
    }

private:
    friend class GameState;
    int mNumLines;
    int mNumSingles;
    int mNumDoubles;
    int mNumTriples;
    int mNumTetrises;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATESTATS_H
