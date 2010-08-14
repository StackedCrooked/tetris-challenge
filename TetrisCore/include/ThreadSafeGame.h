#ifndef THREADSAFEGAME_H_INCLUDED
#define THREADSAFEGAME_H_INCLUDED


#include "Game.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>


namespace Tetris
{

    class Game;

    /**
     * ThreadSafeGame
     *
     * Takes ownership of a Game object and pairs it with a mutex.
     * The Game object is stored in a shared_ptr so that you can create
     * multiple copies of the same ThreadSafeGame object.
     *
     * Users of this class should always first lock the mutex before
     * accessing the Game pointer.
     *
     * Example usage:
     *
     * // Assuming "threadSafeGame" is pointer to a ThreadSafeGame object.
     * // Create an additional scope to define the scope of the scoped_lock object.
     * {
     *     // First lock the game object.
     *     boost::mutex::scoped_lock lock(threadSafeGame->getMutex());
     *
     *     // Then access it.
     *     Game * game = threadSafeGame->getGame();
     *     game->move(Direction_Down);
     * }
     *
     */
    class ThreadSafeGame
    {
    public:
        ThreadSafeGame(std::auto_ptr<Game> inGame);

        const Game * getGame() const;

        Game * getGame();

        boost::mutex & getMutex() const;

    private:
        boost::shared_ptr<Game> mGame;
        mutable boost::mutex mMutex;
    };

} // namespace Tetris


#endif // THREADSAFEGAME_H_INCLUDED
