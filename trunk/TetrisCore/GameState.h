#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Block.h"
#include "GenericGrid.h"
#include <set>
#include <vector>


namespace Tetris
{


    class GameState
    {
    public:
        GameState();

        const Grid & grid() const;

        Grid & grid();

        int numLines() const;

        int numDoubles() const;

        int numTriples() const;

        int numTetrises() const;

        // Calculates the quality of the playing field.
        // Caches the value.
        int quality() const;

        // When most blocks have been dropped in the field sometimes
        // the remaining openings don't fit a tetris block anymore
        // (for example an opening consisting of 2 or 3 holes). This
        // makes it impossible to finish the puzzle, and thus is an
        // indicator that the current game state is a dead end.
        bool hasTopHoles() const;

        // Checks if a block can be placed at a given location without
        // overlapping with previously placed blocks.
        bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

        // Creates a copy of this gamestate with an added block at the given location.
        GameState makeGameStateWithAddedBlock(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

        // Returns the number of holes. A hole is an empty square is covered by blocks.
        int numHoles() const;

    private:
        // For a given square, counts the number of neighor squares
        // that have the same value. This is a helper method for
        // the method 'hasTopHoles'.
        int countNeighbors(size_t inRowIdx, size_t inColIdx) const;

        Grid mGrid;
        int mNumLines;
        int mNumSingles;
        int mNumDoubles;
        int mNumTriples;
        int mNumTetrises;

        // Do we need to recalculate the score?
        mutable bool mDirty;
        mutable int mCachedScore;
        mutable int mNumHoles;

        // check for top holes
        static GenericGrid<int> sHelperGrid;
        static int sMarker;
    };


} // namespace Tetris


#endif // GAMESTATE_H
