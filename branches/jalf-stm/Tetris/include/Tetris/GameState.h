#ifndef TETRIS_GAMESTATE_H_INCLUDED
#define TETRIS_GAMESTATE_H_INCLUDED


#include "Tetris/Block.h"
#include "Tetris/GameOver.h"
#include "Tetris/Grid.h"
#include <memory>
#include <stdexcept>


namespace Tetris {


class GameState
{
public:
    GameState(unsigned inRowCount, unsigned inColumnCount);

    GameState(const GameState & inGameState, const Block & inBlock, GameOver inGameOver);

    inline GameState commit(const Block & inBlock, GameOver inGameOver) const
    { return GameState(*this, inBlock, inGameOver); }

    const Grid & grid() const;

    // Modifies the grid bypassing Tetris rules.
    // This is required to enable certain multiplayer features.
    void setGrid(const Grid & inGrid);

    // Returns true if setGrid() has been called on this object.
    bool tainted() const;

    // The Block that was used in the commit(...) call.
    const Block & originalBlock() const;

    bool isGameOver() const;

    // Checks if a Block can be placed at a given location
    // without overlapping with previously placed blocks.
    bool checkPositionValid(const Block & inBlock, unsigned inRowIdx, unsigned inColIdx) const;


    // Statistics
    int numLines() const { return mNumLines; }
    int numSingles() const { return mNumSingles; }
    int numDoubles() const { return mNumDoubles; }
    int numTriples() const { return mNumTriples; }
    int numTetrises() const { return mNumTetrises; }
    int score() const;
    int firstOccupiedRow() const { return mFirstOccupiedRow; }
    int currentHeight() const { return mGrid.rowCount() - mFirstOccupiedRow; }

    void updateCache();

private:
    void solidifyBlock(const Block & inBlock);
    void clearLines();

    Grid mGrid;
    Block mOriginalBlock;
    bool mIsGameOver;
    unsigned mFirstOccupiedRow;
    int mNumLines;
    int mNumSingles;
    int mNumDoubles;
    int mNumTriples;
    int mNumTetrises;
    bool mTainted;
};


class EvaluatedGameState
{
public:
    // Takes ownership of the GameState object
    EvaluatedGameState(const GameState & inGameState, signed inQuality);

    ~EvaluatedGameState();

    const GameState & gameState() const;

    GameState & gameState();

    signed quality() const;

private:
    EvaluatedGameState(const EvaluatedGameState &);
    EvaluatedGameState& operator=(const EvaluatedGameState&);

    GameState mGameState;
    int mQuality;
};


} // namespace Tetris


#endif // GAMESTATE_H_INCLUDED
