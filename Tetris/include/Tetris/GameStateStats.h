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
                   int inNumTetrises) :
        mNumLines(inNumLines),
        mNumSingles(inNumSingles),
        mNumDoubles(inNumDoubles),
        mNumTriples(inNumTriples),
        mNumTetrises(inNumTetrises)
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

private:
    int mNumLines;
    int mNumSingles;
    int mNumDoubles;
    int mNumTriples;
    int mNumTetrises;
    int mFirstOccupiedRow;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATESTATS_H_INCLUDED
