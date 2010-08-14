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

    private:
        boost::scoped_ptr<Game> mGame;
        mutable boost::mutex mMutex;
    };

} // namespace Tetris

#endif // THREADSAFEGAME_H_INCLUDED
