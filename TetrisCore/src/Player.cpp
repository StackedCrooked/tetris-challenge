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
        //Poco::Mutex::ScopedLock lock(mThreadCountMutex);
        //mThreadCount = inThreadCount;
        //print("Thread count is now: " + boost::lexical_cast<std::string>(getThreadCount()));
    }

    
    size_t Player::getThreadCount() const
    {
        //Poco::Mutex::ScopedLock lock(mThreadCountMutex);
        //return mThreadCount;
        return 0;
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
            // Thread-fest.
            mPlayer.populateNodeMultiThreaded(mNode, mBlocks, mSelectionCounts);
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
        
        std::vector<boost::shared_ptr<Poco::Runnable> > runnables;
        std::vector<boost::shared_ptr<Poco::Thread> > threads;
        

        while (selection < inSelectionCounts.front() && it != children.end())
        {
            ChildNodePtr childNode = *it;
            threads.push_back(boost::shared_ptr<Poco::Thread>(new Poco::Thread));
            threads.back()->setPriority(Poco::Thread::PRIO_HIGH);
            runnables.push_back(boost::shared_ptr<Poco::Runnable>(new ThreadedPopulator(*this, *childNode, DropFirst(inBlocks), DropFirst(inSelectionCounts))));
            threads.back()->start(*runnables.back());
            selection++;
            ++it;
        }
        setThreadCount(getThreadCount() + threads.size());

        // Wait for all thread to finish.
        for (size_t idx = 0; idx != threads.size(); ++idx)
        {
            threads[idx]->join();
            setThreadCount(getThreadCount() - 1);
        }
    }


    void Player::move(const std::vector<int> & inSelectionCounts)
    {
        populateNodeMultiThreaded(mGame->currentNode(), mGame->getFutureBlocks(inSelectionCounts.size()), inSelectionCounts);
    }


    void Player::playUntilGameOver(const std::vector<int> & inDepths)
    {
        while (!mGame->isGameOver())
        {
            move(inDepths);
            size_t depth = inDepths.size();
            GameStateNode * child = mGame->currentNode().bestChild(depth);
            while (!child && depth > 0)
            {
                child = mGame->currentNode().bestChild(depth--);
            }

            GameStateNode * immediateChildParent = child->parent();
            GameStateNode * immediateChild = child;
            while (immediateChild->depth() - mGame->currentNode().depth() > 0)
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

            mGame->setCurrentNode(child);
        }
    }


} // namespace Tetris
