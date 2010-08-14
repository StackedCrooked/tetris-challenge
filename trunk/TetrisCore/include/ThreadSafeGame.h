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
    struct GameAndMutex
    {
        GameAndMutex(std::auto_ptr<Game> inGame) : mGame(inGame.release()) { }
        boost::scoped_ptr<Game> mGame;
        mutable boost::mutex mMutex;
    };


    class ThreadSafeGame
    {
    public:
        ThreadSafeGame(std::auto_ptr<Game> inGame) :
            mGameAndMutex(new GameAndMutex(inGame))
        {
        }
    private:
        friend class WritableGame;
        friend class ReadOnlyGame;
        boost::shared_ptr<GameAndMutex> mGameAndMutex;
    };


    class WritableGame
    {
    public:
        WritableGame(ThreadSafeGame & inThreadSafeGame) :
            mLock(inThreadSafeGame.mGameAndMutex->mMutex),
            mGame(inThreadSafeGame.mGameAndMutex->mGame.get())
        {
        }

        Game * operator->()
        {
            return mGame;
        }
    private:
        boost::mutex::scoped_lock mLock;
        Game * mGame;
    };


    class ReadOnlyGame
    {
    public:
        ReadOnlyGame(const ThreadSafeGame & inThreadSafeGame) :
            mLock(inThreadSafeGame.mGameAndMutex->mMutex),
            mGame(inThreadSafeGame.mGameAndMutex->mGame.get())
        {
        }

        const Game * operator->() const
        {
            return mGame;
        }
    private:
        boost::mutex::scoped_lock mLock;
        const Game * mGame;
    };

} // namespace Tetris


#endif // THREADSAFEGAME_H_INCLUDED
