#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include "Poco/Stopwatch.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <ostream>


namespace Tetris
{
    
    
    // Remove all the children from srcNode except the one on the path that leads to dstNode.
    // The children of the 'good one' are also exterminated, except the one that brings us a 
    // step closer to dstNode. This algorithm is recursively repeated until only the path between
    // srcNode and dstNode remains.
    //
    // The purpose of this function is mainly to free up memory.
    //
    void CarvePath(GameStateNode * srcNode, GameStateNode * dstNode)
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


    //
    // Populates the given node with children. Then it continues to populate each of the children, and their children, ...
    // It ends when it has recursed as many times as (inBlocks.size() - inOffset).
    // The inWidths argument how many children should be kept. This is needed to curb the exponential explosion.
    //
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
            PopulateNodesRecursively(inNode, inBlocks, inWidths, inOffset + 1);
            ++idx;
            ++it;
        }
    }


    Player::Player(Game * inGame) :
        mGame(inGame),
        mOutStream(0)
    {
    }


    void PopulateChildNodes_Mt(const ChildNodes & ioChildNodes,
                               const BlockTypes & inBlocks,
                               const std::vector<int> & inWidths,
                               boost::thread_group & ioThreadPool)
    {
        // These variables must not be destroyed until after the 'threadPool.join_all()' below.
        std::vector<boost::shared_ptr<BlockTypes> > keepAlive_BlockTypes;
        std::vector<boost::shared_ptr<Widths> > keepAlive_Widths;
        std::vector<boost::shared_ptr<GameStateNode> > keepAlive_GameStateNodes;

        int count = 0;
        const int cMaxChildCount = inWidths[0];
        ChildNodes::const_iterator it = ioChildNodes.begin(), end = ioChildNodes.end();
        while (it != end && count != cMaxChildCount)
        {
            GameStateNode & node = **it;
            
            // WARNING!
            // The PopulateNodesRecursively method takes const ref arguments. We must make sure
            // any arguments lifetime will last until after the threadPool.join_all call below.

            // One BIG advantage however is that each thread will have its own copy of the data structures.
            // This leads to greatly reduced interthread synchronization, which in turn leads to much
            // more efficient CPU utilization and thus better performance.
            keepAlive_BlockTypes.push_back(boost::shared_ptr<BlockTypes>(new BlockTypes(inBlocks)));
            keepAlive_Widths.push_back(boost::shared_ptr<Widths>(new Widths(inWidths)));
            
            // Call the PopulateNodesRecursively function in a separate thread.
            std::auto_ptr<GameStateNode> clonedGameState(node.clone());
            ioThreadPool.create_thread(
                boost::bind(&PopulateNodesRecursively,
                            clonedGameState.get(),
                            *keepAlive_BlockTypes.back(),
                            *keepAlive_Widths.back(),
                            1));
            
            // Give owner-ship of the cloned game-state to the keepAlive object.
            keepAlive_GameStateNodes.push_back(boost::shared_ptr<GameStateNode>(clonedGameState.release()));
            count++;
            ++it;
        }

        // Wait for all threads to complete.
        ioThreadPool.join_all();
    }


    void Move(GameStateNode * ioStartingNode,
              const BlockTypes & inBlockTypes,
              const Widths & inWidths,
              boost::thread_group & ioThreadPool)
    {
        CheckPrecondition(!inWidths.empty(), "Player::move: depth should be at least 1.");
        CheckPrecondition(!inBlockTypes.empty(), "Number of future blocks must be at least one.");

        // Generate the offspring.
        ChildNodes & childNodes = ioStartingNode->children();
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
                                  ioThreadPool);
        }
    }


    void Player::move(const Widths & inWidths)
    {
        boost::thread_group threadPool;
        BlockTypes blockTypes;
        mGame->getFutureBlocks(inWidths.size(), blockTypes);
        Move(mGame->currentNode(), blockTypes, inWidths, threadPool);
        threadPool.join_all();
    }


    void Player::playUntilGameOver(const std::vector<int> & inWidths)
    {
        Poco::Stopwatch stopWatch;
        stopWatch.start();

        int printHelper = mGame->currentNode()->depth();
        while (!mGame->isGameOver())
        {
            move(inWidths);
            size_t depth = inWidths.size();
            GameStateNode * child = mGame->currentNode()->bestChild(depth);
            while (!child && depth > 0) // in case of game-over this can happen
            {
                child = mGame->currentNode()->bestChild(depth--);
            }
            
            CarvePath(mGame->currentNode(), child);
            mGame->setCurrentNode(child);

            int durationMs = (int)(stopWatch.elapsed() / 1000);
            if (durationMs > 500)
            {
                log("Blocks: " + boost::lexical_cast<std::string>(mGame->currentNode()->depth()) + "\tLines: " + boost::lexical_cast<std::string>(mGame->currentNode()->state().stats().numLines()));
                printHelper = mGame->currentNode()->depth();
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


} // namespace Tetris
