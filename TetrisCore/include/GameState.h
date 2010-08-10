#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED


#include "Block.h"
#include "Grid.h"
#include <memory>


namespace Tetris
{

    class GameState
    {
    public:
        // Creates a new GameState
        GameState(int inNumRows, int inNumColumns);

        const Grid & grid() const;

        Grid & grid();

        // The Block that was used in the commit(...) call.
        const Block & originalBlock() const;

        bool isGameOver() const;

        // Calculates the quality of the playing field.
        // Caches the value.
        int quality() const;

        // Checks if a activeBlock can be placed at a given location without
        // overlapping with previously placed blocks.
        bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

        // Creates a copy of the current gamestate with the given active block committed.
        // Use inGameOver = true to mark the new gamestate as "game over".
        std::auto_ptr<GameState> commit(const Block & inBlock, GameOver inGameOver) const;

        std::auto_ptr<GameState> clone() const;

        class Stats
        {
        public:
            Stats() :
                mNumLines(0),
                mNumSingles(0),
                mNumDoubles(0),
                mNumTriples(0),
                mNumTetrises(0)
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
            friend class GameState;
            int mNumLines;
            int mNumSingles;
            int mNumDoubles;
            int mNumTriples;
            int mNumTetrises;
        };

        const Stats & stats() const;

    private:
        void solidifyBlock(const Block & inBlock);

        void clearLines();

        Grid mGrid;
        Stats mStats;
        bool mIsGameOver;
        Block mOriginalBlock;

        class Quality
        {
        public:
            Quality() :
                mIsInitialized(false),
                mScore(0),
                mNumHoles(0)
            {
            }

            bool isInitialized() const
            { return mIsInitialized; }

            void setInitialized(bool inIsInitialized)
            { mIsInitialized = inIsInitialized; }

            int score() const
            { return mScore; }

            void setScore(int inScore)
            { mScore = inScore; }

            int numHoles() const
            { return mNumHoles; }

            void setNumHoles(int inNumHoles)
            { mNumHoles = inNumHoles; }

            void reset()
            {
                mScore = 0;
                mNumHoles = 0;
            }

        private:
            bool mIsInitialized;
            int mScore;
            int mNumHoles;
        };

        mutable Quality mQuality;
    };


} // namespace Tetris


#endif // GAMESTATE_H_INCLUDED

