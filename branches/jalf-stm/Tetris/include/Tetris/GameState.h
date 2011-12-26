#ifndef TETRIS_GAMESTATE_H
#define TETRIS_GAMESTATE_H


#include "Tetris/Block.h"
#include "Tetris/Grid.h"
#include "Tetris/GameStateStats.h"
#include <memory>
#include <stdexcept>


namespace Tetris {


class GameState
{
public:
    GameState(unsigned inRowCount, unsigned inColumnCount);

    GameState commit(const Block & inBlock) const;

    inline unsigned id() const
    { return mId; }

    inline const Grid & grid() const
    { return mGrid; }

    // Modifies the grid bypassing Tetris rules.
    // This is required to enable certain multiplayer features.
    void setGrid(const Grid & inGrid);

    // Returns true if setGrid() has been called on this object.
    inline bool tainted() const
    { return mTainted; }

    // The Block that was used in the commit(...) call.
    inline const Block & originalBlock() const
    { return mOriginalBlock; }

    inline bool isGameOver() const
    { return mIsGameOver; }

    // Checks if a Block can be placed at a given location
    // without overlapping with previously placed blocks.
    bool checkPositionValid(const Block & inBlock, unsigned inRowIdx, unsigned inColIdx) const;

    inline bool checkPositionValid(const Block & inBlock) const
    { return checkPositionValid(inBlock, inBlock.row(), inBlock.column()); }

    inline int numLines() const
    { return mStats.mNumLines; }

    inline int numSingles() const
    { return mStats.mNumSingles; }

    inline int numDoubles() const
    { return mStats.mNumDoubles; }

    inline int numTriples() const
    { return mStats.mNumTriples; }

    inline int numTetrises() const
    { return mStats.mNumTetrises; }

    inline int score() const
    { return mStats.score(); }

    inline int firstOccupiedRow() const
    { return mFirstOccupiedRow; }

    inline int currentHeight() const
    { return mGrid.rowCount() - mFirstOccupiedRow; }

private:
    void solidifyBlock(const Block & inBlock);
    void clearLines();
    void updateCache();

    Grid mGrid;
    Block mOriginalBlock;
    bool mIsGameOver;
    unsigned mFirstOccupiedRow;
    GameStateStats mStats;
    bool mTainted;
    unsigned mId;
};


class EvaluatedGameState
{
public:
    // Takes ownership of the GameState object
    EvaluatedGameState(const GameState & inGameState, signed inQuality);

    ~EvaluatedGameState();

    const GameState & gameState() const;

    GameState & gameState();

    inline signed quality() const { return mQuality; }

private:
    EvaluatedGameState(const EvaluatedGameState &);
    EvaluatedGameState& operator=(const EvaluatedGameState&);

    GameState mGameState;
    int mQuality;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATE_H
