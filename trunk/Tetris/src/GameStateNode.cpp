#include "Tetris/Config.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
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

        int identifier() const;

        const Evaluator & evaluator() const;

        int depth() const;

        const NodePtr parent() const;

        NodePtr parent();

        void makeRoot();

        const ChildNodes & children() const;

        ChildNodes & children();

        void clearChildren();

        void addChild(NodePtr inChildNode);

        const GameState & state() const;

        GameState & state();

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
        mChildren(GameStateComparator(mEvaluator->clone()))
    {
    }


    GameStateNodeImpl::GameStateNodeImpl(std::auto_ptr<GameState> inGameState,
                                         std::auto_ptr<Evaluator> inEvaluator) :
        mParent(),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(0),
        mGameState(inGameState),
        mEvaluator(inEvaluator.release()),
        mChildren(GameStateComparator(mEvaluator->clone()))
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


    int GameStateNodeImpl::identifier() const
    {
        return mIdentifier;
    }


    const Evaluator & GameStateNodeImpl::evaluator() const
    {
        return *mEvaluator;
    }


    const GameState & GameStateNodeImpl::state() const
    {
        return *mGameState;
    }


    GameState & GameStateNodeImpl::state()
    {
        return *mGameState;
    }


    ChildNodes & GameStateNodeImpl::children()
    {
        return mChildren;
    }


    const ChildNodes & GameStateNodeImpl::children() const
    {
        return mChildren;
    }


    void GameStateNodeImpl::clearChildren()
    {
        mChildren.clear();
    }


    void GameStateNodeImpl::addChild(NodePtr inChildNode)
    {
        Assert(inChildNode->depth() == mDepth + 1);
        mChildren.insert(inChildNode);
    }


    NodePtr GameStateNodeImpl::parent()
    {
        return mParent.lock();
    }


    int GameStateNodeImpl::depth() const
    {
        return mDepth;
    }


    const NodePtr GameStateNodeImpl::parent() const
    {
        return mParent.lock();
    }


    void GameStateNodeImpl::makeRoot()
    {
        mParent.reset();
    }


    //
    // GameStateNode definitions
    //
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
        return mImpl->identifier();
    }


    const Evaluator & GameStateNode::evaluator() const
    {
        return mImpl->evaluator();
    }


    const GameState & GameStateNode::state() const
    {
        return mImpl->state();
    }


    GameState & GameStateNode::state()
    {
        return mImpl->state();
    }


    ChildNodes & GameStateNode::children()
    {
        return mImpl->children();
    }


    const ChildNodes & GameStateNode::children() const
    {
        return mImpl->children();
    }


    void GameStateNode::clearChildren()
    {
        mImpl->clearChildren();
    }


    void GameStateNode::addChild(NodePtr inChildNode)
    {
        mImpl->addChild(inChildNode);
    }


    const GameStateNode * GameStateNode::endNode() const
    {
        if (mImpl->mChildren.empty())
        {
            return this;
        }
        return (*mImpl->children().begin())->endNode();
    }


    GameStateNode * GameStateNode::endNode()
    {
        if (mImpl->mChildren.empty())
        {
            return this;
        }
        return (*mImpl->children().begin())->endNode();
    }


    NodePtr GameStateNode::parent()
    {
        return mImpl->parent();
    }


    const NodePtr GameStateNode::parent() const
    {
        return mImpl->parent();
    }


    int GameStateNode::depth() const
    {
        return mImpl->depth();
    }


    void GameStateNode::makeRoot()
    {
        mImpl->makeRoot();
    }

} // namespace Tetris
