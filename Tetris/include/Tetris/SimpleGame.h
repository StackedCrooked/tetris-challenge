#ifndef TETRIS_SIMPLEGAME_H_INCLUDED
#define TETRIS_SIMPLEGAME_H_INCLUDED


#include "Tetris/Direction.h"
#include "Tetris/Block.h"
#include "Tetris/GameStateStats.h"
#include "Tetris/PlayerType.h"
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <vector>


namespace Tetris {


class Game;
class GameState;
template<class Variable> class ThreadSafe;


/**
 * SimpleGame is an easy to use and thread-safe wrapper for the Game class.
 */
class SimpleGame
{
public:
    class EventHandler
    {
    public:
        EventHandler() {}

        virtual void onGameStateChanged(SimpleGame * inGame) = 0;

        virtual void onLinesCleared(SimpleGame * inGame, int inLineCount) = 0;

        virtual void onDestroy(SimpleGame * inGame) = 0;

    private:
        EventHandler(const EventHandler&);
        EventHandler& operator=(const EventHandler&);
    };

    class BackReference
    {
    public:
        virtual ~BackReference() {}
    };

    SimpleGame(size_t inRowCount, size_t inColumnCount, PlayerType inPlayerType);

    ~SimpleGame();

    void setBackReference(BackReference * inBackReference);

    template<class SubType>
    SubType & backReference()
    {
        if (!getBackReference())
        {
            throw std::logic_error("BackReference is null");
        }
        return dynamic_cast<SubType&>(*this);
    }

    template<class SubType>
    const SubType & backReference() const
    {
        if (!getBackReference())
        {
            throw std::logic_error("BackReference is null");
        }
        return dynamic_cast<const SubType&>(*this);
    }

    BackReference * getBackReference();

    const BackReference * getBackReference() const;

    PlayerType playerType() const;

    static void RegisterEventHandler(SimpleGame * inSimpleGame, EventHandler * inEventHandler);

    static void UnregisterEventHandler(SimpleGame * inSimpleGame, EventHandler * inEventHandler);

    static bool Exists(SimpleGame * inSimpleGame);

    GameStateStats stats() const;

    ThreadSafe<Game> game() const;

    bool isGameOver() const;

    int rowCount() const;

    int columnCount() const;

    void move(MoveDirection inDirection);

    void rotate();

    void drop();

    int level() const;

    // Returns a copy to avoid race conditions.
    Block activeBlock() const;

    // Returns a copy to avoid race conditions.
    Grid gameGrid() const;

    // Gets the currently active block and any blocks that follow.
    Block getNextBlock() const;

    // For multiplayer crazyness.
    void applyLinePenalty(int inNumberOfLinesMadeByOpponent);
    void setActiveBlock(const Block & inBlock);
    void setGameGrid(const Grid & inGrid);

private:
    // non-copyable
    SimpleGame(const SimpleGame & );
    SimpleGame & operator=(const SimpleGame&);

    struct Impl;
    Impl * mImpl;

    typedef std::set<SimpleGame*> Instances;
    static Instances sInstances;
};


} // namespace Tetris


#endif // TETRIS_SIMPLEGAME_H_INCLUDED
