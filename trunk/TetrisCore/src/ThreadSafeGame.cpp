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


    int ThreadSafeGame::numRows() const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mGame->numRows();
    }


    int ThreadSafeGame::numColumns() const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mGame->numColumns();
    }


    void ThreadSafeGame::reserveBlocks(size_t inCount)
    {
        boost::mutex::scoped_lock lock(mMutex);
        mGame->reserveBlocks(inCount);
    }

        
    const Block & ThreadSafeGame::activeBlock() const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mGame->activeBlock();
    }


    Block & ThreadSafeGame::activeBlock()    
    {        
        boost::mutex::scoped_lock lock(mMutex);
        return mGame->activeBlock();
    }
    
    
    void ThreadSafeGame::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
    {
        boost::mutex::scoped_lock lock(mMutex);
        mGame->getFutureBlock(inCount, outBlocks);
    }


    GameStateNode * ThreadSafeGame::currentNode()
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mGame->currentNode();
    }


    const GameStateNode * ThreadSafeNode::currentNode() const
    {
        boost::mutex::lock lock(mMutex);
        return mGame->currentNode();
    }

       
    void ThreadSafeNode::setCurrentNode(GameStateNode * inCurrentNode)
    {
        boost::mutex::lock lock(mMutex);
        mGame->setCurrentNode(inCurrentNode);
    }


    bool ThreadSafeNode::isGameOver() const
    {
        boost::mutex::lock lock(mMutex);
        return mGame->isGameOver();
    }


    bool ThreadSafeNode::move(Direction inDirection)
    {
        boost::mutex::lock lock(mMutex);
        return mGame->move(inDirection);
    }


    bool ThreadSafeNode::rotate()
    {
        boost::mutex::lock lock(mMutex);
        return mGame->rotate();
    }


    bool ThreadSafeNode::drop() const
    {
        boost::mutex::lock lock(mMutex);
        return mGame->drop();
    }


    bool ThreadSafeNode::navigateNodeUp() const
    {
        boost::mutex::lock lock(mMutex);
        return mGame->navigateNodeUp();
    }


    bool ThreadSafeNode::navigateNodeDown() const
    {
        boost::mutex::lock lock(mMutex);
        return mGame->navigateNodeDown();
    }


    bool ThreadSafeNode::navigateNodeLeft() const
    {
        boost::mutex::lock lock(mMutex);
        return mGame->navigateNodeLeft();
    }


    bool ThreadSafeNode::navigateNodeRight() const
    {
        boost::mutex::lock lock(mMutex);
        return mGame->navigateNodeRight();
    }

} // namespace Tetris

