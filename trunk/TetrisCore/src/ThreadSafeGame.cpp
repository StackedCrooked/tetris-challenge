#include "ThreadSafeGame.h"


namespace Tetris
{

    ThreadSafeGame::ThreadSafeGame(std::auto_ptr<Game> inGame) :
        mGame(inGame.release())
    {
    }


    const Game * ThreadSafeGame::getGame() const
    {
        return mGame.get();
    }

    
    Game * ThreadSafeGame::getGame()
    {
        return mGame.get();
    }


    boost::mutex & ThreadSafeGame::getMutex() const
    {
        return mMutex;
    }


} // namespace Tetris
