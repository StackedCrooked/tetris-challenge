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


namespace Tetris {


static int GetIdentifier(const GameState & inGameState)
{
    const Block & block = inGameState.originalBlock();
    return block.numRotations() * block.column() + block.rotation();
}


struct GameStateNode::Impl
{
    Impl(NodePtr inParent, GameState *  inGameState, Evaluator *  inEvaluator) :
        mParent(inParent),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(inParent->depth() + 1),
        mEvaluatedGameState(new EvaluatedGameState(inGameState, inEvaluator->evaluate(*inGameState))),
        mEvaluator(inEvaluator),
        mChildren(GameStateComparator(mEvaluator->clone()))
    {
    }

    Impl(GameState *  inGameState, Evaluator * inEvaluator) :
        mParent(),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(0),
        mEvaluatedGameState(new EvaluatedGameState(inGameState, inEvaluator->evaluate(*inGameState))),
        mEvaluator(inEvaluator),
        mChildren(GameStateComparator(mEvaluator->clone()))
    {
    }

    boost::weak_ptr<GameStateNode> mParent;
    int mIdentifier;
    int mDepth;
    boost::scoped_ptr<EvaluatedGameState> mEvaluatedGameState;
    boost::scoped_ptr<Evaluator> mEvaluator; // }
    ChildNodes mChildren;                    // } => Order matters!
};


std::auto_ptr<GameStateNode> GameStateNode::CreateRootNode(size_t inNumRows, size_t inNumColumns)
{
    return std::auto_ptr<GameStateNode>(new GameStateNode(
        new GameState(inNumRows, inNumColumns),
        new Balanced));
}


GameStateNode::GameStateNode(GameState *  inGameState, Evaluator *  inEvaluator) :
    mImpl(new Impl(inGameState, inEvaluator))
{
}


GameStateNode::GameStateNode(NodePtr inParent, GameState *  inGameState, Evaluator *  inEvaluator) :
    mImpl(new Impl(inParent, inGameState, inEvaluator))
{
}


GameStateNode::~GameStateNode()
{
    delete mImpl;
    mImpl = 0;
}


std::auto_ptr<GameStateNode> GameStateNode::clone() const
{
    NodePtr parent = mImpl->mParent.lock();
    std::auto_ptr<GameStateNode> result(parent ? new GameStateNode(parent, new GameState(mImpl->mEvaluatedGameState->gameState()), mImpl->mEvaluator->clone().release())
                                               : new GameStateNode(new GameState(mImpl->mEvaluatedGameState->gameState()), mImpl->mEvaluator->clone().release()));
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
    return *mImpl->mEvaluator;
}


const GameState & GameStateNode::gameState() const
{
    return mImpl->mEvaluatedGameState->gameState();
}


void GameStateNode::setGrid(const Grid & inGrid)
{
    clearChildren();
    GameState copy = mImpl->mEvaluatedGameState->gameState();
    copy.setGrid(inGrid);
    copy.updateCache();
    int score = mImpl->mEvaluator->evaluate(copy);
    mImpl->mIdentifier = GetIdentifier(copy);
    mImpl->mEvaluatedGameState.reset(new EvaluatedGameState(new GameState(copy), score));
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
