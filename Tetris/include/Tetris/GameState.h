#ifndef TETRIS_GAMESTATE_H
#define TETRIS_GAMESTATE_H


#include "Tetris/Block.h"
#include "Tetris/Grid.h"
#include "Tetris/GameStateStats.h"
#include "Futile/Assert.h"
#include <boost/bind.hpp>
#include <algorithm>
#include <memory>
#include <stdexcept>


namespace Tetris {


class GameState
{
public:
    GameState(unsigned inRowCount, unsigned inColumnCount);

    GameState commit(const Block & inBlock) const;

    unsigned id() const
    { return mId; }

    const Grid & grid() const
    { return mGrid; }

    // Modifies the grid bypassing Tetris rules.
    // This is required to enable certain multiplayer features.
    void setGrid(const Grid & inGrid);

    // Returns true if setGrid() has been called on this object.
    bool tainted() const
    { return mTainted; }

    // The Block that was used in the commit(...) call.
    const Block & originalBlock() const
    { return mOriginalBlock; }

    bool isGameOver() const
    { return mIsGameOver; }

    // Checks if a Block can be placed at a given location
    // without overlapping with previously placed blocks.
    bool checkPositionValid(const Block & inBlock, unsigned inRowIdx, unsigned inColIdx) const;

    bool checkPositionValid(const Block & inBlock) const
    { return checkPositionValid(inBlock, inBlock.row(), inBlock.column()); }

    int numLines() const
    { return mStats.mNumLines; }

    int numSingles() const
    { return mStats.mNumSingles; }

    int numDoubles() const
    { return mStats.mNumDoubles; }

    int numTriples() const
    { return mStats.mNumTriples; }

    int numTetrises() const
    { return mStats.mNumTetrises; }

    int score() const
    { return mStats.score(); }

    int firstOccupiedRow() const
    { return mFirstOccupiedRow; }

    int currentHeight() const
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


//! Wraps a std::vector<GameState> and performs consistency checking.
class GameStateList
{
public:
    GameStateList() {}

    GameStateList(const std::vector<GameState> & inGameStates) :
        mGameStates(inGameStates)
    {
    }

    void push_back(const GameState & inGameState)
    {
        #ifndef NDEBUG
        if (!mGameStates.empty())
        {
            const GameState & gs = mGameStates.back();
            auto id1 = gs.id();
            auto id2 = inGameState.id();
            Assert(id1 + 1 == id2);
        }
        #endif
        mGameStates.push_back(inGameState);
    }

    void push_front(const GameState & inGameState)
    {
        Assert(mGameStates.empty() || inGameState.id() + 1 == mGameStates.front().id());
        mGameStates.push_back(inGameState);
        std::sort(mGameStates.begin(),
                  mGameStates.end(),
                  boost::bind(&GameState::id, _1) < boost::bind(&GameState::id, _2));
    }

    void pop_back() { mGameStates.pop_back(); }

    void pop_front() { mGameStates.erase(mGameStates.begin()); }

    void clear() { mGameStates.clear(); }

    const GameState & back() const { return mGameStates.back(); }
    GameState & back() { return mGameStates.back(); }

    const GameState & front() const { return mGameStates.front(); }
    GameState & front() { return mGameStates.front(); }

    std::size_t size() const { return mGameStates.size(); }

    bool empty() const { return mGameStates.empty(); }

    const std::vector<GameState> & get() const { return mGameStates; }

    std::vector<GameState>::iterator begin() { return mGameStates.begin(); }
    std::vector<GameState>::iterator end() { return mGameStates.end(); }

    std::vector<GameState>::const_iterator begin() const { return mGameStates.begin(); }
    std::vector<GameState>::const_iterator end() const { return mGameStates.end(); }

private:
    std::vector<GameState> mGameStates;
};


struct GameOver : public std::runtime_error
{
    GameOver(const GameState & inGameState) :
        std::runtime_error("Game Over"),
        mGameState(inGameState)
    {
    }

    virtual ~GameOver() throw() {}

    const GameState & gameState() const { return mGameState; }

private:
    GameState mGameState;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATE_H
