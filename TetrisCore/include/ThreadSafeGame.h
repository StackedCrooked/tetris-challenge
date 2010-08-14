#ifndef THREADSAFEGAME_H_INCLUDED
#define THREADSAFEGAME_H_INCLUDED


#include "Game.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <memory>


namespace Tetris
{

    class Game;

    /**
     * ThreadSafeGame
     *
     */
    class ThreadSafeGame
    {
    public:
        ThreadSafeGame(std::auto_ptr<Game> inGame);

        // A function that can modify the game object
        typedef boost::function<void(Game*)> Action;

        void doto(const Action & inAction);        

        int numRows() const;

        int numColumns() const;

        void reserveBlocks(size_t inCount);

        const Block & activeBlock() const;

        Block & activeBlock();

        // Includes the currently active block
        void getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const;

        GameStateNode * currentNode();

        const GameStateNode * currentNode() const;

        void setCurrentNode(GameStateNode * inCurrentNode);

        bool isGameOver() const;

        //
        // Game commands
        //
        bool move(Direction inDirection);

        void rotate();

        void drop();

        //
        // Navigate the game history and alternative histories.
        // Experimental.
        //
        bool navigateNodeUp();

        bool navigateNodeDown();

        bool navigateNodeLeft();

        bool navigateNodeRight();

    private:
        boost::scoped_ptr<Game> mGame;
        mutable boost::mutex mMutex;
    };



} // namespace Tetris


#endif // THREADSAFEGAME_H_INCLUDED
