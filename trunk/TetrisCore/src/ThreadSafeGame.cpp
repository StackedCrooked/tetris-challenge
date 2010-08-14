#include "ThreadSafeGame.h"


namespace Tetris
{

    ThreadSafeGame::ThreadSafeGame(std::auto_ptr<Game> inGame) :
        mGame(inGame.release())
    {
    }


    void ThreadSafeGame::doto(const Action & inAction)
    {
        boost::mutex::scoped_lock lock(mMutex);
        inAction(mGame.get());
    }

} // namespace Tetris
