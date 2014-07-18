#ifndef TETRIS_GAME_H_INCLUDED
#define TETRIS_GAME_H_INCLUDED


#include "Tetris/BlockFactory.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/GameState.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include "Futile/Threading.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <set>


namespace Tetris {


class Block;
class GameStateNode;
class Game;


/**
 * Game is the base class for HumanGame and ComputerGame subclasses.
 *
 * It manages the following things:
 *   - the currently active block
 *   - the list of future blocks
 *   - the root gamestate node
 */
class Game
{
protected:
    /**
     * Constructor is private. Use the factory methods defined in subtype.
     */
    Game(std::size_t inNumRows, std::size_t inNumColumns);

    // Friendship required for destructor.
    friend class Futile::ThreadSafe<Game>;

public:
    virtual ~Game();

    boost::signals2::signal<void(Game *)> GameStateChanged;
    boost::signals2::signal<void(Game *, int)> LinesCleared;

    void setPaused(bool inPause);

    bool isPaused() const;

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    bool canMove(MoveDirection inDirection);

    virtual bool move(MoveDirection inDirection) = 0;

    bool rotate();

    void dropWithoutCommit();

    void dropAndCommit();

    int level() const;

    void setStartingLevel(int inLevel);

    const Block & activeBlock() const;

    const Grid & gameGrid() const;

    std::size_t currentBlockIndex() const;

    void getFutureBlocks(std::size_t inCount, BlockTypes & outBlocks) const;

    void getFutureBlocksWithOffset(std::size_t inOffset, std::size_t inCount, BlockTypes & outBlocks) const;

    virtual const GameState & gameState() const = 0;

    // Multiplayer feature: add 1, 2 or 4 penalty lines after
    // an opponent cleared a combo of 2, 3 or 4 lines respectively.
    virtual void applyLinePenalty(std::size_t inLineCount);
    
    // Set the grid of this player.
    // Can be used for bonus or penalty features.
    virtual void setGrid(const Grid & inGrid) = 0;

protected:
    static int GetRowDelta(MoveDirection inDirection);
    static int GetColumnDelta(MoveDirection inDirection);

    virtual GameState & gameState() = 0;

    void onChanged();
    void onLinesCleared(std::size_t inLineCount);

    static std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, std::size_t inNumColumns);
    void reserveBlocks(std::size_t inCount);
    void supplyBlocks();

    std::vector<BlockType> getGarbageRow() const;

    std::size_t mNumRows;
    std::size_t mNumColumns;
    boost::scoped_ptr<Block> mActiveBlock;
    boost::scoped_ptr<BlockFactory> mBlockFactory;
    boost::scoped_ptr<BlockFactory> mGarbageFactory;
    mutable BlockTypes mBlocks;
    std::size_t mCurrentBlockIndex;
    int mStartingLevel;
    bool mPaused;

    // In order to avoid flooding the queue in certain situations.
    bool mMuteEvents;

    /**
     * Create an instance to make the mMuteEvents false over a certain scope.
     * This is handy when we are running a loop and don't want to trigger for
     * for each event.
     */
    struct ScopedMute : boost::noncopyable
    {
        ScopedMute(bool & value) :
            mValue(value)
        {
            mValue = true;
        }

        ~ScopedMute()
        {
            mValue = false;
        }

        bool & mValue;
    };

private:
    // non-copyable
    Game(const Game&);
    Game& operator=(const Game&);
};


class HumanGame : public Game
{
public:
    inline static Futile::ThreadSafe<Game> Create(std::size_t inNumRows, std::size_t inNumColumns)
    {
        return Futile::ThreadSafe<Game>(new HumanGame(inNumRows, inNumColumns));
    }

    virtual bool move(MoveDirection inDirection);

    const GameState & gameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    // Friendship required for constructor.
    friend class Game;

    HumanGame(std::size_t inNumRows, std::size_t inNumCols);

    HumanGame(const Game & inGame);

    GameState & gameState();

private:
    boost::scoped_ptr<GameState> mGameState;
};


class ComputerGame : public Game
{
public:
    inline static Futile::ThreadSafe<Game> Create(std::size_t inNumRows, std::size_t inNumColumns)
    {
        return Futile::ThreadSafe<Game>(new ComputerGame(inNumRows, inNumColumns));
    }

    virtual bool move(MoveDirection inDirection);

    void appendPrecalculatedNode(NodePtr inNode);

    const GameStateNode * currentNode() const;

    const GameStateNode * endNode() const;

    bool navigateNodeDown();

    std::size_t numPrecalculatedMoves() const;

    void clearPrecalculatedNodes();

    const GameState & gameState() const;

    virtual void setGrid(const Grid & inGrid);

protected:
    // Friendship required for constructor.
    friend class Game;

    ComputerGame(std::size_t inNumRows, std::size_t inNumCols);

    ComputerGame(const Game & inGame);

    GameState & gameState();

private:
    void setCurrentNode(NodePtr inCurrentNode);

    NodePtr mCurrentNode;
};


} // namespace Tetris


#endif // GAME_H_INCLUDED
