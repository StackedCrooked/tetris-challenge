#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include <boost/signals2.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>


namespace Tetris {


class Block;
class GameStateNode;
class GameImpl;


/**
 * Game is a top-level class for the Tetris core. It manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - the root gamestate node
 */
class Game
{
public:
    boost::signals2::signal<void(Game&)> OnChanged;

    Game(size_t inNumRows, size_t inNumColumns);

    virtual ~Game();

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    virtual bool move(MoveDirection inDirection) = 0;

    bool rotate();

    void drop();

    int level() const;

    void setLevel(int inLevel);

    const Block & activeBlock() const;

    const Grid & gameGrid() const;

    size_t currentBlockIndex() const;

    void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

    void getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const;

    virtual const GameState & getGameState() const = 0;

    // For multiplayer crazyness
    virtual void setActiveBlock(const Block & inBlock);
    virtual void setGrid(const Grid & inGrid) = 0;
    void swapGrid(Game & other);
    void swapActiveBlock(Game & other);

protected:
    virtual GameState & getGameState() = 0;

    static std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns);
    void reserveBlocks(size_t inCount);
    void supplyBlocks() const;
    void setDirty();

    size_t mNumRows;
    size_t mNumColumns;
    boost::scoped_ptr<Block> mActiveBlock;
    boost::scoped_ptr<BlockFactory> mBlockFactory;
    mutable BlockTypes mBlocks;
    size_t mCurrentBlockIndex;
    int mOverrideLevel;

    mutable boost::mutex mChangedSignalMutex;

private:
    // non-copyable
    Game(const Game&);
    Game& operator=(const Game&);
};


class HumanGame : public Game
{
public:
    HumanGame(size_t inNumRows, size_t inNumCols);

    HumanGame(const Game & inGame);

    virtual bool move(MoveDirection inDirection);

    const GameState & getGameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    GameState & getGameState();

private:
    boost::scoped_ptr<GameState> mGameState;
};


class ComputerGame : public Game
{
public:
    ComputerGame(size_t inNumRows, size_t inNumCols);

    ComputerGame(const Game & inGame);

    virtual bool move(MoveDirection inDirection);

    void appendPrecalculatedNode(NodePtr inNode);

    const GameStateNode * currentNode() const;

    const GameStateNode * lastPrecalculatedNode() const;

    bool navigateNodeDown();

    size_t numPrecalculatedMoves() const;

    void clearPrecalculatedNodes();

    const GameState & getGameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    GameState & getGameState();

private:
    void setCurrentNode(NodePtr inCurrentNode);

    NodePtr mCurrentNode;
};


} // namespace Tetris


#endif // GAME_H_INCLUDED
