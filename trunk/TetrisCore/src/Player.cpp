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


    // Remove all the children from srcNode except the one on the path that leads to dstNode.
    // The children of the 'good one' are also exterminated, except the one that brings us a 
    // step closer to dstNode. This algorithm is recursively repeated until only the path between
    // srcNode and dstNode remains.
    //
    // The purpose of this function is mainly to free up memory.
    //
    void DestroyInferiorChildren(GameStateNode & startNode, GameStateNode & endNode)
    {
        GameStateNode * dst = &endNode;
        assert(dst->depth() > startNode.depth());
        while (dst->depth() != startNode.depth())
        {
            ChildNodes::const_iterator it = dst->parent()->children().begin(), end = dst->parent()->children().end();
            for (; it != end; ++it)
            {
                ChildNodePtr endNode = *it;
                if (endNode.get() == dst) // is endNode part of the path between startNode and endNode?
                {
                    // Erase all children. The endNode object is kept alive
                    // thanks to the refcounting mechanism of boost::shared_ptr.
                    dst->parent()->clearChildren();

                    // Add endNode to the child nodes again.
                    dst->parent()->addChild(endNode);
                    break;
                }
            }
            dst = dst->parent();
        }
    }


    Player::Player(std::auto_ptr<GameStateNode> inNode,
                   const BlockTypes & inBlockTypes) :
        mNode(inNode.release()),
        mEndNode(),
        mBlockTypes(inBlockTypes),
        mIsFinished(false),
        mThread()
    {
        assert(mNode->children().empty());
    }


    void Player::populateNodesRecursively(GameStateNode & ioNode, const BlockTypes & inBlockTypes, size_t inDepth)
    {
        if (inDepth >= inBlockTypes.size())
        {
            return;
        }
        
        if (ioNode.state().isGameOver())
        {
            // Game over state has no children.
            return;
        }

        // Generate the child nodes
        ChildNodes generatedChildNodes;
        GenerateOffspring(inBlockTypes[inDepth], ioNode, generatedChildNodes);        
        ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
        for (; it != end; ++it)
        {
            ChildNodePtr child = *it;
            ioNode.addChild(child);
            if (inDepth == inBlockTypes.size() - 1)
            {
                if (!mEndNode)
                {
                    mEndNode = child;
                }
                else if (child->state().quality() > mEndNode->state().quality())
                {
                    mEndNode = child;
                }
            }
        }

        // Recursion.
        for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
        {
            populateNodesRecursively(**it, inBlockTypes, inDepth + 1);
        }
    }
    
        
    bool Player::isFinished() const
    {
        return mIsFinished;
    }


    ChildNodePtr Player::result()
    {
        assert(!mNode->children().empty());
        return *mNode->children().begin();
    }


    void Player::start()
    {
        assert(!mThread);
        if (!mThread)
        {
            mThread.reset(new boost::thread(boost::bind(&Player::startImpl, this)));
        }
    }


    void Player::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            populateNodesRecursively(*mNode, mBlockTypes, 0);
            if (mEndNode)
            {
                DestroyInferiorChildren(*mNode, *mEndNode);
            }
            mIsFinished = true;
        } 
        catch (const std::exception & inException)
        {
            LogError(MakeString() << inException.what());
        }
    }


} // namespace Tetris
