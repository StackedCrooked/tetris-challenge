#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED


#include "Grid.h"
#include <memory>


namespace Tetris
{

    class Block;
    class ActiveBlock;

    class GameState
    {
    public:
        // Creates a new GameState
        GameState(int inNumRows, int inNumColumns);

        const Grid & grid() const;

        Grid & grid();

        // The ActiveBlock that was used in the commit(...) call.
        const ActiveBlock * originalActiveBlock() const;

        int numLines() const;

        int numDoubles() const;

        int numTriples() const;

        int numTetrises() const;

        bool isGameOver() const;

        // Calculates the quality of the playing field.
        // Caches the value.
        int quality() const;

        // Checks if a activeBlock can be placed at a given location without
        // overlapping with previously placed blocks.
        bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

        // Returns the number of holes. A hole is an empty square is covered by blocks.
        int numHoles() const;

        // Creates a copy of the current gamestate with the given active block committed.
        // Use inGameOver = true to mark the new gamestate as "game over".
        std::auto_ptr<GameState> commit(std::auto_ptr<ActiveBlock> inBlock, bool inGameOver) const;

        std::auto_ptr<GameState> clone() const;

    private:
        void solidifyBlock(const ActiveBlock * inActiveBlock);

        void clearLines();

        struct Stats
        {
            Stats() :
                mNumLines(0),
                mNumSingles(0),
                mNumDoubles(0),
                mNumTriples(0),
                mNumTetrises(0)
            {
            }

            int mNumLines;
            int mNumSingles;
            int mNumDoubles;
            int mNumTriples;
            int mNumTetrises;
        };

        Grid mGrid;
        Stats mStats;
        bool mIsGameOver;
        std::auto_ptr<ActiveBlock> mOriginalActiveBlock;

        struct Quality
        {
            Quality() :
                mDirty(true),
                mScore(0),
                mNumHoles(0)
            {
            }

            bool mDirty;
            int mScore;
            int mNumHoles;
        };

        mutable Quality mQuality;
    };


} // namespace Tetris


#endif // GAMESTATE_H_INCLUDED

