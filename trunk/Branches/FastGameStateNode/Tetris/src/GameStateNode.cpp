#include "Tetris/Config.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/Utilities.h"
#include "Futile/Assert.h"
#include "Futile/Playground/NNode.h"
#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace Tetris {


using namespace Futile;


static int GetIdentifier(const GameState & inGameState)
{
    const Block & block = inGameState.originalBlock();
    return block.numRotations() * block.column() + block.rotation();
}


struct GameStateNodeData
{
    enum {
        N = 5,
        H = 8,
        MaxDepth = 8,
        RowCount = 20,   // }
        ColumnCount = 10 // } => For now put these here
    };

    GameStateNodeData() :
        mNodeBase(),
        mGameStateNode(new GameStateNode(new GameState(RowCount, ColumnCount), MakeTetrises::Instance()))
    {
    }


    NNodeBase<GameStateNodeData, N, H> * mNodeBase;
    boost::shared_ptr<GameStateNode> mGameStateNode;
};


struct GameStateNode::Impl
{
    Impl(GameState * inGameState, const Evaluator & inEvaluator) :
        mParent(),
        mGameState(inGameState),
        mEvaluator(inEvaluator),
        mNodeData(),
        mIdentifier(GetIdentifier(*inGameState)),
        mScore(),
        mDepth(0),
        mChildren()
    {
    }

    Impl(NodePtr inParent, GameState *  inGameState, const Evaluator &  inEvaluator) :
        mParent(inParent),
        mGameState(inGameState),
        mEvaluator(inEvaluator),
        mNodeData(),
        mIdentifier(GetIdentifier(*inGameState)),
        mScore(),
        mDepth(inParent->depth() + 1),
        mChildren()
    {
    }

    boost::weak_ptr<GameStateNode> mParent;
    boost::scoped_ptr<GameState> mGameState;
    const Evaluator & mEvaluator;
    GameStateNodeData * mNodeData;
    int mIdentifier;
    int mScore;
    int mDepth;
    ChildNodes mChildren;
};


typedef RootNode<GameStateNodeData, GameStateNodeData::N, GameStateNodeData::H> FastGameStateNode;
typedef NNodeBase<GameStateNodeData, GameStateNodeData::N, GameStateNodeData::H> BaseNode;
typedef NNode<GameStateNodeData, GameStateNodeData::N, GameStateNodeData::H, GameStateNodeData::H> LeafNode;


template<typename T, unsigned N, unsigned H>
void InitNodeData(NNode<T, N, H, 0> & node)
{
    GameStateNodeData & data = node.mData;
    data.mNodeBase = &node;

    // Has a parent (because D == 0)
    data.mGameStateNode.reset(new GameStateNode(new GameState(GameStateNodeData::RowCount, GameStateNodeData::ColumnCount), MakeTetrises::Instance().Instance()));

    GameStateNode & gameStateNode = *data.mGameStateNode.get();
    GameStateNode::Impl & impl = *gameStateNode.mImpl;
    impl.mDepth = node.Depth;
    impl.mScore = impl.mEvaluator.evaluate(*impl.mGameState);
}


template<typename T, unsigned N, unsigned H, unsigned D>
void InitNodeData(NNode<T, N, H, D> & node)
{
    GameStateNodeData & data = node.mData;
    data.mNodeBase = &node;

    // Has a parent (because D > 0)
    GameStateNodeData & parentData = node.parent()->mData;
    data.mGameStateNode.reset(parentData.mGameStateNode->clone().release());

    GameStateNode & gameStateNode = *data.mGameStateNode.get();
    GameStateNode::Impl & impl = *gameStateNode.mImpl;
    impl.mDepth = node.Depth;
    impl.mScore = impl.mEvaluator.evaluate(*impl.mGameState);
}


/**
 * Leaf nodes are processed here.
 */
template<typename T, unsigned N, unsigned H>
void InitNode(NNode<T, N, H, H> & node)
{
    InitNodeData(node);
}


/**
 * Non-leaf nodes are processed here
 */
template<typename T, unsigned N, unsigned H, unsigned D>
void InitNode(NNode<T, N, H, D> & node)
{
    InitNodeData(node);
    for (std::size_t idx = 0; idx < node.ChildCount; ++idx)
    {
        Futile::NNode<T, N, H, D + 1> & child = node.mChildren[idx];
        InitNode(child);
        GameStateNodeData & nodeData = node.mData;
        GameStateNodeData & childData = child.mData;
        nodeData.mGameStateNode->mImpl->mChildren.insert(childData.mGameStateNode);
    }
}


FastGameStateNode & GetGameStateTree()
{
    static boost::scoped_ptr<FastGameStateNode> fGameStateTree;
    if (!fGameStateTree)
    {
        fGameStateTree.reset(new FastGameStateNode);
        FastGameStateNode & root = *fGameStateTree;
        InitNode(root);
    }
    return *fGameStateTree;
}


std::auto_ptr<GameStateNode> GameStateNode::CreateRootNode(std::size_t inNumRows, std::size_t inNumColumns)
{
    return std::auto_ptr<GameStateNode>(new GameStateNode(
        new GameState(inNumRows, inNumColumns),
        Balanced::Instance()));
}


GameStateNode::GameStateNode(GameState * inGameState, const Evaluator &  inEvaluator) :
    mImpl(new Impl(inGameState, inEvaluator))
{
}


GameStateNode::GameStateNode(NodePtr inParent, GameState *  inGameState, const Evaluator &  inEvaluator) :
    mImpl(new Impl(inParent, inGameState, inEvaluator))
{
}


GameStateNode::~GameStateNode()
{
    mImpl.reset();
}


std::auto_ptr<GameStateNode> GameStateNode::clone() const
{
    NodePtr parent = mImpl->mParent.lock();
    std::auto_ptr<GameStateNode> result(parent ? new GameStateNode(parent, new GameState(gameState()), mImpl->mEvaluator)
                                               : new GameStateNode(new GameState(gameState()), mImpl->mEvaluator));
    result->mImpl->mDepth = mImpl->mDepth;

    ChildNodes::const_iterator it = mImpl->mChildren.begin(), end = mImpl->mChildren.end();
    for (; it != end; ++it)
    {
        GameStateNode & node(*(*it));
        NodePtr newChild(node.clone().release());
        Assert(newChild->depth() == mImpl->mDepth + 1);
        result->mImpl->mChildren.insert(newChild);
    }
    return result;
}


int GameStateNode::identifier() const
{
    return mImpl->mIdentifier;
}


const Evaluator & GameStateNode::evaluator() const
{
    return mImpl->mEvaluator;
}


const GameState & GameStateNode::gameState() const
{
    return *mImpl->mGameState;
}


int GameStateNode::quality() const
{
    return mImpl->mScore;
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


NodePtr GameStateNode::parent()
{
    return mImpl->mParent.lock();
}


const NodePtr GameStateNode::parent() const
{
    return mImpl->mParent.lock();
}


int GameStateNode::depth() const
{
    return mImpl->mDepth;
}


void GameStateNode::makeRoot()
{
    mImpl->mParent.reset();
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


} // namespace Tetris
