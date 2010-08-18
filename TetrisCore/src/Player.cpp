#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include "Logger.h"
#include "Poco/Stopwatch.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <ostream>


namespace Tetris
{


    GameStateNode * FindBestChild(GameStateNode * start, size_t inDepth)
    {
        if (start->children().empty())
        {
            return start;
        }

        GameStateNode * child = start->bestChild(inDepth);
        while (!child && inDepth > 0) // in case of game-over this can happen
        {
            child = start->bestChild(inDepth--);
        }
        return child;
    }

    
    // Remove all the children from srcNode except the one on the path that leads to dstNode.
    // The children of the 'good one' are also exterminated, except the one that brings us a 
    // step closer to dstNode. This algorithm is recursively repeated until only the path between
    // srcNode and dstNode remains.
    //
    // The purpose of this function is mainly to free up memory.
    //
    void DestroyInferiorChildren(GameStateNode * srcNode, GameStateNode * dstNode)
    {
        CheckPrecondition(srcNode != NULL, "Goddamn.");
        CheckPrecondition(dstNode != NULL, "Goddamn.");
        GameStateNode * dstParent = dstNode->parent();
        GameStateNode * dst = dstNode;
        while (dst->depth() - srcNode->depth() > 0)
        {
            ChildNodes::iterator it = dstParent->children().begin(), end = dstParent->children().end();
            for (; it != end; ++it)
            {
                ChildNodePtr dstNode = *it;
                if (dstNode.get() == dst) // is dstNode part of the path between srcNode and dstNode?
                {
                    // Erase all children. The dstNode object is kept alive
                    // thanks to the refcounting mechanism of boost::shared_ptr.
                    dstParent->children().clear();

                    // Add dstNode to the child nodes again.
                    dstParent->children().insert(dstNode);
                    break;
                }
            }
            dst = dst->parent();
            dstParent = dst->parent();
        }
    }


    void DestroyInferiorChildren(GameStateNode * start, size_t inDepth)
    {
        CheckCondition(start != 0, "Start node must not be null.");
        GameStateNode * child = FindBestChild(start, inDepth);
        CheckCondition(child != 0, "FindBestChild must not return null.");
        DestroyInferiorChildren(start, child);
    }


    // Populates the given node with children. Then it continues to populate
    // each of the children, and their children, ... until the we have reached
    // the requested depth.
    void PopulateNodesRecursively(GameStateNode * inNode,
                                  const BlockTypes & inBlocks,
                                  const std::vector<int> & inWidths,
                                  size_t inOffset)
    {

        CheckArgument(inBlocks.size() == inWidths.size(), "PopulateNodesRecursively got inBlocks.size() != inWidths.size()");
        if (inOffset >= inBlocks.size())
        {
            return;
        }

        
        if (inNode->state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        // Generate the child nodes
        GenerateOffspring(inBlocks[inOffset], *inNode, inNode->children());

        // Populate each child node.
        int idx = 0;
        const int cMaxChilds = inWidths[inOffset];
        ChildNodes & children = inNode->children();
        ChildNodes::iterator it = children.begin();
        while (idx < cMaxChilds && it != children.end())
        {
            GameStateNode & childNode = **it;
            PopulateNodesRecursively(&childNode, inBlocks, inWidths, inOffset + 1);
            ++idx;
            ++it;
        }
    }


    Player::Player(const ThreadSafeGame & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mOutStream(0)
    {
    }


    struct ClonedData
    {
        ClonedData(std::auto_ptr<GameStateNode> inNode, const BlockTypes & inBlockTypes, const Widths & inWidths) :
            node(inNode.release()),
            blockTypes(inBlockTypes),
            widths(inWidths)
        {
        }
        boost::shared_ptr<GameStateNode> node;
        BlockTypes blockTypes;
        Widths widths;
    };

    typedef boost::shared_ptr<ClonedData> ClonedDataPtr;
    typedef std::vector<ClonedDataPtr> ClonedDatas;


    void PopulateChildNodes_Mt(const ChildNodes & inChildNodes,
                               const BlockTypes & inBlocks,
                               const std::vector<int> & inWidths,
                               ClonedDatas & ioClonedDatas,
                               boost::thread_group & outThreadPool)
    {
        int count = 0;
        const int cMaxChildCount = inWidths[0];
        ChildNodes::const_iterator it = inChildNodes.begin(), end = inChildNodes.end();
        while (it != end && count != cMaxChildCount)
        {
            GameStateNode & node = **it;
            
            // Clone the data, to be passed to the new thread
            boost::shared_ptr<ClonedData> clonedData(new ClonedData(node.clone(), inBlocks, inWidths));
            ioClonedDatas.push_back(clonedData);
            
            // Call the PopulateNodesRecursively function in a separate thread.
            std::auto_ptr<GameStateNode> clonedGameState(node.clone());
            outThreadPool.create_thread(
                boost::bind(&PopulateNodesRecursively,
                            clonedData->node.get(),
                            clonedData->blockTypes,
                            clonedData->widths,
                            1));
            count++;
            ++it;
        }
    }


    void Move(GameStateNode * ioStartingNode,
              const BlockTypes & inBlockTypes,
              const Widths & inWidths,
              ClonedDatas & ioClonedDatas,
              boost::thread_group & outThreadPool)
    {
        CheckPrecondition(!inWidths.empty(), "Player::move: depth should be at least 1.");
        CheckPrecondition(!inBlockTypes.empty(), "Number of future blocks must be at least one.");

        // Generate the offspring.
        ChildNodes & childNodes = ioStartingNode->children();
        CheckCondition(childNodes.empty(), "Child nodes should be empty before generating its offspring.");
        GenerateOffspring(inBlockTypes[0], *ioStartingNode, childNodes);

        // If the dept is only 1, then we take this shortcut.
        if (inWidths.size() == 1)
        {
            GameStateNode & node = *ioStartingNode;
            
            // Keep-alive
            ChildNodePtr bestChild = *node.children().begin();

            // Erase all children, 'bestChild' is kept alive by shared_ptr.
            node.children().clear();

            node.children().insert(bestChild);
            return;
        }
        else
        {
            PopulateChildNodes_Mt(childNodes,
                                  inBlockTypes,
                                  inWidths,
                                  ioClonedDatas,
                                  outThreadPool);
        }
    }


    void Player::move(const Widths & inWidths)
    {
        // This will keep alive the thread datas.
        ClonedDatas clonedDatas;

        std::auto_ptr<GameStateNode> clonedCurrentNode;

        // Limit scope of thread_group
        {            
            boost::thread_group threadPool;
            BlockTypes blockTypes;

            // Start critical section
            {
                WritableGame game(mThreadSafeGame);
                clonedCurrentNode = game->currentNode()->clone();
                game->getFutureBlocks(inWidths.size(), blockTypes);                
            }
            
            Move(clonedCurrentNode.get(), blockTypes, inWidths, clonedDatas, threadPool);
            threadPool.join_all();
        }

        WritableGame game(mThreadSafeGame);
        ChildNodes::iterator it = clonedCurrentNode->children().begin(), end = clonedCurrentNode->children().end();
        size_t idx = 0;
        for (; idx != clonedDatas.size() && it != end; ++it, ++idx)
        {
            GameStateNode & node = **it;
            ClonedData & data = *clonedDatas[idx];
            game->currentNode()->children().insert(data.node);
        }
        DestroyInferiorChildren(game->currentNode(), inWidths.size());
        game->setCurrentNode(FindBestChild(game->currentNode(), inWidths.size()));
    }


    void Player::playUntilGameOver(const std::vector<int> & inWidths)
    {
        Poco::Stopwatch stopWatch;
        stopWatch.start();

        int printHelper = ReadOnlyGame(mThreadSafeGame)->currentNode()->depth();
        while (!ReadOnlyGame(mThreadSafeGame)->isGameOver())
        {
            move(inWidths);

            int durationMs = (int)(stopWatch.elapsed() / 1000);
            if (durationMs > 500)
            {
                ReadOnlyGame game(mThreadSafeGame);
                log("Blocks: " + boost::lexical_cast<std::string>(game->currentNode()->depth()) + "\tLines: " + boost::lexical_cast<std::string>(game->currentNode()->state().stats().numLines()));
                printHelper = game->currentNode()->depth();
                stopWatch.restart();
            }
        }
    }


    void Player::setLogger(std::ostream & inOutStream)
    {
        mOutStream = &inOutStream;
    }

    
    void Player::log(const std::string & inMessage)
    {
        if (mOutStream)
        {
            *mOutStream << inMessage << "\r";
        }
    }


    TimedNodePopulator::TimedNodePopulator(std::auto_ptr<GameStateNode> inNode,
                                           const BlockTypes & inBlockTypes,
                                           int inTimeMs) :
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mTimeMicroseconds(1000 * inTimeMs)
    {
    }


    ChildNodePtr TimedNodePopulator::getBestChild() const
    {
        if (mFlattenedNodes.empty())
        {
            throw std::runtime_error("No child nodes were generated? This is exceptionally unlikely!");
        }
        return *mFlattenedNodes.back().begin();
    }


    void TimedNodePopulator::addToFlattenedNodes(ChildNodePtr inChildNode, size_t inOffset)
    {
        boost::mutex::scoped_lock lock(mFlattenedNodesMutex);
        while (inOffset >= mFlattenedNodes.size())
        {
            mFlattenedNodes.push_back(ChildNodes());
        }
        mFlattenedNodes[inOffset].insert(inChildNode);
    }


    bool TimedNodePopulator::isTimeExpired() const
    {
        boost::mutex::scoped_lock lock(mStopwatchMutex);
        return mStopwatch.elapsed() > mTimeMicroseconds;
    }


    void TimedNodePopulator::populateNodesRecursively(GameStateNode & ioNode, const BlockTypes & inBlockTypes, size_t inOffset)
    {
        if (isTimeExpired())
        {
            return;
        }

        if (inOffset >= inBlockTypes.size())
        {
            return;
        }
        
        if (ioNode.state().isGameOver())
        {
            // Game over state has no children.
            return;
        }

        // Generate the child nodes
        GenerateOffspring(inBlockTypes[inOffset], ioNode, ioNode.children());        
        ChildNodes & children = ioNode.children();

        // Store the fruit of our labor.
        for (ChildNodes::iterator it = children.begin(); it != children.end(); ++it)
        {
            addToFlattenedNodes(*it, inOffset);
        }

        // Recursion.
        for (ChildNodes::iterator it = children.begin(); it != children.end(); ++it)
        {
            GameStateNode & childNode = **it;
            populateNodesRecursively(childNode, inBlockTypes, inOffset + 1);
        }
    }


    void TimedNodePopulator::populateNodesInBackground(GameStateNode * inNode, BlockTypes * inBlockTypes, size_t inOffset)
    {
        // Thread entry point
        try
        {
            boost::scoped_ptr<BlockTypes> takeOwnership(inBlockTypes);
            populateNodesRecursively(*inNode, *inBlockTypes, inOffset);
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "populateNodesInBackground failed: " << inException.what());
        }
    }


    void TimedNodePopulator::start()
    {
        mStopwatch.start();

        // Generate children of the root node
        ChildNodes childNodes;
        GenerateOffspring(mBlockTypes[0], *mNode, childNodes);

        // For each child node:
        ChildNodes::const_iterator it = childNodes.begin(), end = childNodes.end();
        for (; it != end; ++it)
        {
            GameStateNode & firstGenerationChildNode = **it;
            mThreadPool.create_thread(
                boost::bind(&TimedNodePopulator::populateNodesInBackground,
                            this,
                            &firstGenerationChildNode,  // maybe clone here (I'm not sure...)
                            new BlockTypes(mBlockTypes),
                            1));
        }

        mThreadPool.join_all();
    }


} // namespace Tetris
