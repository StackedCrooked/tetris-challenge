#ifndef GAMECOMMANDQUEUE_H_INCLUDED
#define GAMECOMMANDQUEUE_H_INCLUDED


#include "ThreadSafeGame.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <list>


namespace Tetris
{

    class GameCommandQueue
    {
    public:
        GameCommandQueue(const ThreadSafeGame & inThreadSafeGame);

        const ThreadSafeGame & threadSafeGame() const;

        ThreadSafeGame & threadSafeGame();

        typedef boost::function<void(Game*)> Action;

        void append(const Action & inAction);

        // Executes the next command in line.
        void executeNextCommand();

    private:
        ThreadSafeGame mThreadSafeGame;
        std::list<Action> mQueue;
        mutable boost::mutex mQueueMutex;
    };

} // namespace Tetris


#endif // GAMECOMMANDQUEUE_H_INCLUDED
