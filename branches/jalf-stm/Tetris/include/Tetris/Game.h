#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <boost/scoped_ptr.hpp>
#include <deque>
#include <functional>
#include <memory>


namespace Tetris {


class Block;
class GameStateNode;
class Game;


/**
 * Game manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - block factory
 *   - paused state
 */
class Game
{
public:
    Game(std::size_t inNumRows, std::size_t inNumColumns);

    ~Game();

    // Threaded!
    boost::signals2::signal<void()> GameStateChanged;

    // Threaded!
    boost::signals2::signal<void(unsigned)> LinesCleared;

    unsigned gameStateId() const;

    void setPaused(bool inPause);

    bool isPaused() const;

    bool isGameOver() const;

    GameStateStats stats() const;

    Grid grid() const;

    Block activeBlock() const;

    int rowCount() const;

    int columnCount() const;

    int level() const;

    bool checkPositionValid(const Block & inBlock) const;

    bool canMove(Direction inDirection) const;

    enum MoveResult
    {
        MoveResult_Moved,
        MoveResult_NotMoved,
        MoveResult_Committed
    };

    virtual MoveResult move(Direction inDirection);

    MoveResult rotate();

    void dropWithoutCommit();

    void dropAndCommit();

    BlockTypes getFutureBlocks(std::size_t inCount);

    int firstOccupiedRow() const;

    void setStartingLevel(int inLevel);

    Grid gameGrid() const;

    virtual void applyLinePenalty(std::size_t inLineCount);

    GameState gameState() const;

private:
    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_GAME_H
