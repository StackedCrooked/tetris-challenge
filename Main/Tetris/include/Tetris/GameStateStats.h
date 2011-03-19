#ifndef TETRIS_GAMESTATESTATS_H_INCLUDED
#define TETRIS_GAMESTATESTATS_H_INCLUDED


namespace Tetris {


class GameStateStats
{
public:
    GameStateStats(int inNumLines,
                   int inNumSingles,
                   int inNumDoubles,
                   int inNumTriples,
                   int inNumTetrises,
                   int inCurrentHeight) :
        mNumLines(inNumLines),
        mNumSingles(inNumSingles),
        mNumDoubles(inNumDoubles),
        mNumTriples(inNumTriples),
        mNumTetrises(inNumTetrises),
        mCurrentHeight(inCurrentHeight)
    {
    }

    inline int numLines() const
    { return mNumLines; }

    inline int numSingles() const
    { return mNumSingles; }

    inline int numDoubles() const
    { return mNumDoubles; }

    inline int numTriples() const
    { return mNumTriples; }

    inline int numTetrises() const
    { return mNumTetrises; }

    inline int currentHeight() const
    { return mCurrentHeight; }

    inline int score() const
    {
        return   40 * mNumSingles +
                100 * mNumDoubles +
                300 * mNumTriples +
               1200 * mNumTetrises;
    }

private:
    int mNumLines;
    int mNumSingles;
    int mNumDoubles;
    int mNumTriples;
    int mNumTetrises;
    int mCurrentHeight;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATESTATS_H_INCLUDED
