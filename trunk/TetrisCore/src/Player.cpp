#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include <boost/shared_ptr.hpp>


namespace Tetris
{


    Player::Player(Game * inGame) :
        mGame(inGame)
    {
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


    // Forward declaration.
    void PopulateNode_MultiThreaded(GameStateNode & inNode, const std::vector<BlockType> & inBlocks, const std::vector<int> & inSelectionCounts);


    class ThreadedPopulator : public Poco::Runnable
    {
    public:
        ThreadedPopulator(GameStateNode & inNode, const std::vector<BlockType> & inBlocks, const std::vector<int> & inSelectionCounts) :
            mNode(inNode),
            mBlocks(inBlocks),
            mSelectionCounts(inSelectionCounts)
        {
            CheckArgument(inBlocks.size() == inSelectionCounts.size(), "PopulateNode got inBlocks.size() != inSelectionCounts.size()");
        }

        virtual void run()
        {
            // Thread-fest.
            PopulateNode_MultiThreaded(mNode, mBlocks, mSelectionCounts);
        }

    private:
        GameStateNode & mNode;
        const std::vector<BlockType> & mBlocks;
        const std::vector<int> & mSelectionCounts;
    };
    

    void PopulateNode_MultiThreaded(GameStateNode & inNode, const std::vector<BlockType> & inBlocks, const std::vector<int> & inSelectionCounts)
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
        
        std::vector<boost::shared_ptr<Poco::Runnable> > runnables;
        std::vector<boost::shared_ptr<Poco::Thread> > threads;
        

        while (selection < inSelectionCounts.front() && it != children.end())
        {
            ChildNodePtr childNode = *it;
            threads.push_back(boost::shared_ptr<Poco::Thread>(new Poco::Thread));
            runnables.push_back(boost::shared_ptr<Poco::Runnable>(new ThreadedPopulator(*childNode, DropFirst(inBlocks), DropFirst(inSelectionCounts))));
            threads.back()->start(*runnables.back());
            selection++;
            ++it;
        }

        // Wait for all thread to finish.
        for (size_t idx = 0; idx != threads.size(); ++idx)
        {
            threads[idx]->join();
        }
    }


    void Player::move(const std::vector<int> & inSelectionCounts)
    {
        PopulateNode_MultiThreaded(mGame->currentNode(), mGame->getFutureBlocks(inSelectionCounts.size()), inSelectionCounts);
    }


} // namespace Tetris
