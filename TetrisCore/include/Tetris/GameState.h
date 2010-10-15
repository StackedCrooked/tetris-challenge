#ifndef TETRIS_GAMESTATE_H_INCLUDED
#define TETRIS_GAMESTATE_H_INCLUDED


#include "Tetris/Tetris.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/Block.h"
#include "Tetris/GenericGrid.h"
#include "Tetris/Grid.h"
#include <memory>


namespace Tetris
{

    class GameState
    {
    public:
        // Creates a new GameState
        GameState(size_t inNumRows, size_t inNumColumns);

        const Grid & grid() const;

        Grid & grid();

        // The Block that was used in the commit(...) call.
        const Block & originalBlock() const;

        bool isGameOver() const;

        // Calculates the quality of the playing field.
        // Caches the value.
        int quality(const Evaluator & inEvaluator) const;

        // Requires that quality has been called once before with an Evaluator argument.
        int quality() const;

        // Checks if a activeBlock can be placed at a given location without
        // overlapping with previously placed blocks.
        bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

        // Creates a copy of the current gamestate with the given active block committed.
        // Use inGameOver = true to mark the new gamestate as "game over".
        std::auto_ptr<GameState> commit(const Block & inBlock, GameOver inGameOver) const;

        class Stats
        {
        public:
            Stats() :
                mNumLines(0),
                mNumSingles(0),
                mNumDoubles(0),
                mNumTriples(0),
                mNumTetrises(0),
                mFirstOccupiedRow(0)
            {
            }

            int score() const;

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

            inline int firstOccupiedRow() const
            { return mFirstOccupiedRow; }

            // Use zero based index!
            inline int numLines(size_t idx) const
            {
                switch (idx)
                {
                    case 0: return mNumSingles;
                    case 1: return mNumDoubles;
                    case 2: return mNumTriples;
                    case 3: return mNumTetrises;
                    default: throw std::invalid_argument("Invalid number of lines scored requested.");
                }
                return 0; // compiler happy
            }

        private:
            friend class GameState;
            int mNumLines;
            int mNumSingles;
            int mNumDoubles;
            int mNumTriples;
            int mNumTetrises;
            int mFirstOccupiedRow;
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

