#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/Utilities.h"
#include "Futile/Assert.h"
#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace Tetris {


static int GetIdentifier(const GameState & inGameState)
{
    const Block & block = inGameState.originalBlock();
    return block.rotationCount() * block.column() + block.rotation();
}


struct GameStateNode::Impl
{
    Impl(NodePtr inParent, const GameState & inGameState, const Evaluator & inEvaluator) :
        mParent(inParent),
        mIdentifier(GetIdentifier(inGameState)),
        mDepth(inParent->depth() + 1),
        mEvaluatedGameState(),
        mEvaluator(inEvaluator),
        mChildren()
    {
        mEvaluatedGameState.reset(new EvaluatedGameState(inGameState, inEvaluator.evaluate(inGameState)));
    }

    Impl(const GameState & inGameState, const Evaluator & inEvaluator) :
        mParent(),
        mIdentifier(GetIdentifier(inGameState)),
        mDepth(0),
        mEvaluatedGameState(),
        mEvaluator(inEvaluator),
        mChildren()
    {
        mEvaluatedGameState.reset(new EvaluatedGameState(inGameState, inEvaluator.evaluate(inGameState)));
    }

    boost::weak_ptr<GameStateNode> mParent;
    int mIdentifier;
    int mDepth;
    boost::scoped_ptr<EvaluatedGameState> mEvaluatedGameState;
    const Evaluator & mEvaluator; // } => Order matters!
    ChildNodes mChildren;         // }    (Evaluator must outlive mChildren)
};


std::unique_ptr<GameStateNode> GameStateNode::CreateRootNode(std::size_t inNumRows, std::size_t inNumColumns)
{
    return std::unique_ptr<GameStateNode>(new GameStateNode(
        GameState(inNumRows, inNumColumns),
        Balanced::Instance()));
}


GameStateNode::GameStateNode(const GameState & inGameState, const Evaluator & inEvaluator) :
    mImpl(new Impl(inGameState, inEvaluator))
{
}


GameStateNode::GameStateNode(NodePtr inParent, const GameState & inGameState, const Evaluator & inEvaluator) :
    mImpl(new Impl(inParent, inGameState, inEvaluator))
{
}


GameStateNode::~GameStateNode()
{
    mImpl.reset();
}


std::unique_ptr<GameStateNode> GameStateNode::clone() const
{
    NodePtr parent = mImpl->mParent.lock();
    std::unique_ptr<GameStateNode> result(parent ? new GameStateNode(parent, gameState(), mImpl->mEvaluator)
                                               : new GameStateNode(gameState(), mImpl->mEvaluator));
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
    return mImpl->mEvaluatedGameState->gameState();
}


int GameStateNode::quality() const
{
    return mImpl->mEvaluatedGameState->quality();
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
