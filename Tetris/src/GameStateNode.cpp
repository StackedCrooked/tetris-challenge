#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/Utilities.h"
#include "Futile/Assert.h"
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
        mGameState(inGameState),
        mDepth(inParent->depth() + 1),
        mScore(inEvaluator.evaluate(inGameState)),
        mEvaluator(inEvaluator),
        mChildren()
    {
    }

    Impl(const GameState & inGameState, const Evaluator & inEvaluator) :
        mParent(),
        mGameState(inGameState),
        mDepth(0),
        mScore(inEvaluator.evaluate(inGameState)),
        mEvaluator(inEvaluator),
        mChildren()
    {
    }

    boost::weak_ptr<GameStateNode> mParent;
    GameState mGameState;
    int mScore;
    int mDepth;
    const Evaluator & mEvaluator; // } => Order matters!


private:
    Impl& operator=(const Impl&);
};


std::unique_ptr<GameStateNode> GameStateNode::CreateRootNode(std::size_t inNumRows, std::size_t inNumColumns)
{
    return std::unique_ptr<GameStateNode>(new GameStateNode(GameState(inNumRows, inNumColumns),
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


GameStateNode::GameStateNode(const GameStateNode & rhs) :
    mImpl(new Impl(*rhs.mImpl))
{
}


GameStateNode::~GameStateNode()
{
    mImpl.reset();
}


const Evaluator & GameStateNode::evaluator() const
{
    return mImpl->mEvaluator;
}


const GameState & GameStateNode::gameState() const
{
    return mImpl->mGameState;
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
