#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>


namespace Tetris
{


    Player::Player(Game * inGame) :
        mGame(inGame),
        mThreadCount(0)
    {
    }


    void Player::print(const std::string & inMessage)
    {
        //Poco::Mutex::ScopedLock lock(mIOMutex);
        //std::cout << inMessage << "\n";
    }
        
        
    void Player::setThreadCount(size_t inThreadCount)
    {
        Poco::Mutex::ScopedLock lock(mThreadCountMutex);
        mThreadCount = inThreadCount;
    }
        
        
    size_t Player::incrementThreadCount()
    {
        Poco::Mutex::ScopedLock lock(mThreadCountMutex);
        return mThreadCount++;
    }
        
        
    size_t Player::decrementThreadCount()
    {
        Poco::Mutex::ScopedLock lock(mThreadCountMutex);
        return mThreadCount--;
    }

    
    size_t Player::getThreadCount() const
    {
        Poco::Mutex::ScopedLock lock(mThreadCountMutex);
        return mThreadCount;
    }


    template<class T>
    std::vector<T> DropFirst(const std::vector<T> & inValues)
    {
        std::vector<T> result;
        for (size_t i = 1; i < inValues.size(); ++i)
        {
            result.push_back(inValues[i]);
        }
        return result;
    }


    void PopulateNode(GameStateNode & inNode, const std::vector<BlockType> & inBlocks, const std::vector<int> & inSelectionCounts)
    {
        CheckArgument(inBlocks.size() == inSelectionCounts.size(), "PopulateNode got inBlocks.size() != inSelectionCounts.size()");
        if (inBlocks.empty())
        {
            return;
        }

        
        if (inNode.state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        // Generate the child nodes for inNode
        GenerateOffspring(inBlocks.front(), inNode, inNode.children());


        
        ChildNodes & children = inNode.children();
        ChildNodes::iterator it = children.begin();
        int selection = 0;
        while (selection < inSelectionCounts.front() && it != children.end())
        {
            ChildNodePtr childNode = *it;
            PopulateNode(*childNode, DropFirst(inBlocks), DropFirst(inSelectionCounts));
            selection++;
            ++it;
        }
    }


    class ThreadedPopulator : public Poco::Runnable
    {
    public:
        ThreadedPopulator(Player & inPlayer, GameStateNode & inNode, const std::vector<BlockType> & inBlocks, const std::vector<int> & inSelectionCounts) :
            mPlayer(inPlayer),
            mNode(inNode),
            mBlocks(inBlocks),
            mSelectionCounts(inSelectionCounts)
        {
            CheckArgument(inBlocks.size() == inSelectionCounts.size(), "PopulateNode got inBlocks.size() != inSelectionCounts.size()");
        }

        virtual void run()
        {
            PopulateNode(mNode, mBlocks, mSelectionCounts);
            delete this;
        }

    private:
        Player & mPlayer;
        GameStateNode & mNode;
        std::vector<BlockType> mBlocks;
        std::vector<int> mSelectionCounts;
    };
    

    void Player::populateNodeMultiThreaded(GameStateNode & inNode, const std::vector<BlockType> & inBlocks, const std::vector<int> & inSelectionCounts)
    {
        print("Thread id: " + boost::lexical_cast<std::string>(Poco::Thread::current() ? Poco::Thread::current()->id() : 0));
        CheckArgument(inBlocks.size() == inSelectionCounts.size(), "PopulateNode got inBlocks.size() != inSelectionCounts.size()");
        if (inBlocks.empty())
        {
            return;
        }

        
        if (inNode.state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        // Generate the child nodes for inNode
        GenerateOffspring(inBlocks.front(), inNode, inNode.children());
        
        ChildNodes & children = inNode.children();
        ChildNodes::iterator it = children.begin();
        int selection = 0;
        
        while (selection < inSelectionCounts.front() && it != children.end())
        {            
            ChildNodePtr childNode = *it;
            if (mThreadPool.used() < mThreadPool.capacity())
            {
                mThreadPool.startWithPriority
                (
                    Poco::Thread::PRIO_HIGH,
                    *(new ThreadedPopulator(*this, *childNode, DropFirst(inBlocks), DropFirst(inSelectionCounts)))
                );
            }
            else
            {
                PopulateNode(*childNode, DropFirst(inBlocks), DropFirst(inSelectionCounts));
            }
            selection++;
            ++it;
        }
    }


    void Player::move(const std::vector<int> & inSelectionCounts, bool inMultiThreaded)
    {
        if (inMultiThreaded)
        {
            populateNodeMultiThreaded(mGame->currentNode(), mGame->getFutureBlocks(inSelectionCounts.size()), inSelectionCounts);            
            mThreadPool.joinAll();
        }
        else
        {
            PopulateNode(mGame->currentNode(), mGame->getFutureBlocks(inSelectionCounts.size()), inSelectionCounts);
        }
    }


    void Player::playUntilGameOver(const std::vector<int> & inDepths, bool inMultiThreaded)
    {
        while (!mGame->isGameOver())
        {
            move(inDepths, inMultiThreaded);
            size_t depth = inDepths.size();
            GameStateNode * child = mGame->currentNode().bestChild(depth);
            while (!child && depth > 0)
            {
                child = mGame->currentNode().bestChild(depth--);
            }
            
            cleanup(&mGame->currentNode(), child);

            mGame->setCurrentNode(child);
        }
    }

    class ThreadedCleaner : public Poco::Runnable
    {
    public:
        ThreadedCleaner(Player * inPlayer, GameStateNode * inCurrentNode, GameStateNode * inChild) :
            mPlayer(inPlayer),
            mCurrentNode(inCurrentNode),
            mChild(inChild)
        {
        }

        virtual void run()
        {
            mPlayer->cleanup(mCurrentNode, mChild);
            delete this;
        }

    private:
        Player * mPlayer;
        GameStateNode * mCurrentNode;
        GameStateNode * mChild;
    };
    

    void Player::cleanup(GameStateNode * currentNode, GameStateNode * child, bool inMultiThreaded)
    {
        if (inMultiThreaded)
        {
            // Not using thread pool here to avoid 
            Poco::Thread * theThread = new Poco::Thread;
            theThread->setPriority(Poco::Thread::PRIO_LOW);
            theThread->start(*(new ThreadedCleaner(this, currentNode, child)));            
        }
        else
        {
            cleanup(currentNode, child);
        }
    }
    
    
    void Player::cleanup(GameStateNode * currentNode, GameStateNode * child)
    {
        GameStateNode * immediateChildParent = child->parent();
        GameStateNode * immediateChild = child;
        while (immediateChild->depth() - currentNode->depth() > 0)
        {
            ChildNodes::iterator it = immediateChildParent->children().begin(), end = immediateChildParent->children().end();
            for (; it != end; ++it)
            {
                ChildNodePtr child = *it;
                if (child.get() == immediateChild)
                {
                    immediateChildParent->children().clear();
                    immediateChildParent->children().insert(child);
                    break;
                }
            }
            immediateChild = immediateChild->parent();
            immediateChildParent = immediateChild->parent();
        }
    }


} // namespace Tetris
