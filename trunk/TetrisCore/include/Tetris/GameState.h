#ifndef TETRIS_GAMESTATE_H_INCLUDED
#define TETRIS_GAMESTATE_H_INCLUDED


#include "Tetris/GameOver.h"
#include "Tetris/Grid.h"
#include <memory>
#include <stdexcept>


namespace Tetris
{

    // Forward declarations    
    class Block;
    class Evaluator;


    // Impl declaration
    class GameStateImpl;
    class Stats;

    
    class GameState
    {
    public:
        // Creates a new GameState
        GameState(size_t inNumRows, size_t inNumColumns);
        
        GameState(const GameState &);

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

        const Stats & stats() const;

    private:
        // not allowed
        GameState& operator=(const GameState&);

        friend class GameStateImpl;
        std::auto_ptr<GameStateImpl> mImpl;
    };


    // GameState stats
    class Stats
    {
    public:
        Stats();

        int score() const;

        int numLines() const;

        int numSingles() const;

        int numDoubles() const;

        int numTriples() const;

        int numTetrises() const;

        int firstOccupiedRow() const;

        // Use zero based index!
        int numLines(size_t idx) const;

    private:
        friend class GameState;
        friend class GameStateImpl;
        int mNumLines;
        int mNumSingles;
        int mNumDoubles;
        int mNumTriples;
        int mNumTetrises;
        int mFirstOccupiedRow;
    };


} // namespace Tetris


#endif // GAMESTATE_H_INCLUDED
