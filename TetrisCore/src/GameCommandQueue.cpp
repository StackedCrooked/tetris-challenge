#include "GameCommandQueue.h"


namespace Tetris
{

    GameCommandQueue::GameCommandQueue(const ThreadSafeGame & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame)
    {
    }


    void GameCommandQueue::append(const Action & inAction)
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        mQueue.push_back(inAction);
    }


    void GameCommandQueue::executeNextCommand()
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        if (!mQueue.empty())
        {
            WritableGame game(mThreadSafeGame);
            mQueue.front()(game.get());
            mQueue.erase(mQueue.begin());
        }
    }


    const ThreadSafeGame & GameCommandQueue::threadSafeGame() const
    {
        return mThreadSafeGame;
    }
    
    
    ThreadSafeGame & GameCommandQueue::threadSafeGame()
    {
        return mThreadSafeGame;
    }

} // namespace Tetris
