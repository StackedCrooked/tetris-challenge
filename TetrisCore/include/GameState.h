#ifndef GAMESTATE_H
#define GAMESTATE_H


#include "GenericGrid.h"
#include "Block.h"
#include <set>
#include <vector>


namespace Tetris
{

    class GameState
    {
    public:
        // Creates a new GameState
        GameState(int inNumRows, int inNumColumns);

        const Grid & grid() const;

        Grid & grid();

        int numLines() const;

        int numDoubles() const;

        int numTriples() const;

        int numTetrises() const;

        // Calculates the quality of the playing field.
        // Caches the value.
        int quality() const;

        // Checks if a activeBlock can be placed at a given location without
        // overlapping with previously placed blocks.
        bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

        // Returns the number of holes. A hole is an empty square is covered by blocks.
        int numHoles() const;

        // Creates a copy of the current gamestate with the given active block committed.        
        std::auto_ptr<GameState> commit(const ActiveBlock & inBlock) const;

    private:
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
    };


} // namespace Tetris


#endif // GAMESTATE_H

