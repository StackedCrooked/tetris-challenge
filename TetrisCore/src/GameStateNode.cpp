#include "Tetris/Config.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/Utilities.h"
#include "Tetris/Assert.h"
#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace Tetris
{
    
    class GameStateNodeImpl
    {
    public:
        GameStateNodeImpl(NodePtr inParent, std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);

        // Creates a root node
        GameStateNodeImpl(std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);
        
        std::auto_ptr<GameStateNode> clone() const;

    private:
        friend class GameStateNode;
        boost::weak_ptr<GameStateNode> mParent;
        int mIdentifier;
        int mDepth;
        boost::scoped_ptr<GameState> mGameState;
        boost::scoped_ptr<Evaluator> mEvaluator; // }
        ChildNodes mChildren;                    // } => Order matters!
    };


    static int GetIdentifier(const GameState & inGameState)
    {
        const Block & block = inGameState.originalBlock();
        return block.numRotations() * block.column() + block.rotation();
    }
    

    GameStateNodeImpl::GameStateNodeImpl(NodePtr inParent,
                                         std::auto_ptr<GameState> inGameState,
                                         std::auto_ptr<Evaluator> inEvaluator) :
        mParent(inParent),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(inParent->depth() + 1),
        mGameState(inGameState),
        mEvaluator(inEvaluator.release()),
        mChildren(GameStateComparisonFunctor(mEvaluator->clone()))
    {
    }


    GameStateNodeImpl::GameStateNodeImpl(std::auto_ptr<GameState> inGameState,
                                         std::auto_ptr<Evaluator> inEvaluator) :
        mParent(),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(0),
        mGameState(inGameState),
        mEvaluator(inEvaluator.release()),
        mChildren(GameStateComparisonFunctor(mEvaluator->clone()))
    {
    }


    std::auto_ptr<GameStateNode> GameStateNodeImpl::clone() const
    {
        NodePtr parent = mParent.lock();
        std::auto_ptr<GameStateNode> result(parent ? new GameStateNode(parent, Create<GameState>(*mGameState), mEvaluator->clone())
                                                   : new GameStateNode(Create<GameState>(*mGameState), mEvaluator->clone()));
        result->mImpl->mDepth = mDepth;

        ChildNodes::const_iterator it = mChildren.begin(), end = mChildren.end();
        for (; it != end; ++it)
        {
            GameStateNode & node(*(*it));
            NodePtr newChild(node.clone().release());
            Assert(newChild->depth() == mDepth + 1);
            result->mImpl->mChildren.insert(newChild);
        }
        return result;
    }


    std::auto_ptr<GameStateNode> GameStateNode::CreateRootNode(size_t inNumRows, size_t inNumColumns)
    {
        return std::auto_ptr<GameStateNode>(new GameStateNode(Create<GameState>(inNumRows, inNumColumns), CreatePoly<Evaluator, Balanced>()));
    }


    GameStateNode::GameStateNode(std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator) :
        mImpl(new GameStateNodeImpl(inGameState, inEvaluator))
    {
    }


    GameStateNode::GameStateNode(NodePtr inParent, std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator) :
        mImpl(new GameStateNodeImpl(inParent, inGameState, inEvaluator))
    {
    }


    GameStateNode::~GameStateNode()
    {
        delete mImpl;
        mImpl = 0;
    }


    std::auto_ptr<GameStateNode> GameStateNode::clone() const
    {
        return mImpl->clone();
    }


    int GameStateNode::identifier() const
    {
        return mImpl->mIdentifier;
    }


    const Evaluator & GameStateNode::evaluator() const
    {
        return *mImpl->mEvaluator;
    }


    const GameState & GameStateNode::state() const
    {
        return *mImpl->mGameState;
    }


    GameState & GameStateNode::state()
    {
        return *mImpl->mGameState;
    }


    ChildNodes & GameStateNode::children()
    {
        return mImpl->mChildren;
    }


    const ChildNodes & GameStateNode::children() const
    {
        return mImpl->mChildren;
    }


    void GameStateNode::clearChildren()
    {
        mImpl->mChildren.clear();
    }


    void GameStateNode::addChild(NodePtr inChildNode)
    {
        Assert(inChildNode->depth() == mImpl->mDepth + 1);
        mImpl->mChildren.insert(inChildNode);
    }


    const GameStateNode * GameStateNode::endNode() const
    {
        if (mImpl->mChildren.empty())
        {
            return this;
        }
        return (*mImpl->mChildren.begin())->endNode();
    }


    GameStateNode * GameStateNode::endNode()
    {
        if (mImpl->mChildren.empty())
        {
            return this;
        }
        return (*mImpl->mChildren.begin())->endNode();
    }


    NodePtr GameStateNode::parent()
    {
        return mImpl->mParent.lock();
    }


    int GameStateNode::depth() const
    {
        return mImpl->mDepth;
    }


    const NodePtr GameStateNode::parent() const
    {
        return mImpl->mParent.lock();
    }


    void GameStateNode::makeRoot()
    {
        mImpl->mParent.reset();
    }

} // namespace Tetris
