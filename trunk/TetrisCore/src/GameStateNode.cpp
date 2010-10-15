#include "Tetris/GameStateNode.h"
#include "Tetris/Assert.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Utilities.h"


namespace Tetris
{


    std::auto_ptr<GameStateNode> GameStateNode::CreateRootNode(size_t inNumRows, size_t inNumColumns)
    {
        return std::auto_ptr<GameStateNode>(new GameStateNode(Create<GameState>(inNumRows, inNumColumns),
                                            CreatePoly<Evaluator, Balanced>()));
    }


    ChildNodes nodes(Balanced);

    static int GetIdentifier(const GameState & inGameState)
    {
        const Block & block = inGameState.originalBlock();
        return block.numRotations() * block.column() + block.rotation();
    }


    GameStateNode::GameStateNode(std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator) :
        mParent(),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(0),
        mGameState(inGameState),
        mEvaluator(inEvaluator.release()),
        mChildren(GameStateComparisonFunctor(mEvaluator->clone()))
    {

    }


    GameStateNode::GameStateNode(NodePtr inParent, std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator) :
        mParent(inParent),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(inParent->depth() + 1),
        mGameState(inGameState),
        mEvaluator(inEvaluator.release()),
        mChildren(GameStateComparisonFunctor(mEvaluator->clone()))
    {
    }


    std::auto_ptr<GameStateNode> GameStateNode::clone() const
    {
        NodePtr parent = mParent.lock();
        std::auto_ptr<GameStateNode> result(parent ? new GameStateNode(parent, Create<GameState>(*mGameState), mEvaluator->clone())
                                                   : new GameStateNode(Create<GameState>(*mGameState), mEvaluator->clone()));
        result->mDepth = mDepth;

        ChildNodes::const_iterator it = mChildren.begin(), end = mChildren.end();
        for (; it != end; ++it)
        {
            GameStateNode & node(*(*it));
            NodePtr newChild(node.clone().release());
            Assert(newChild->depth() == mDepth + 1);
            result->mChildren.insert(newChild);
        }
        return result;
    }


    int GameStateNode::identifier() const
    {
        return mIdentifier;
    }


    const Evaluator & GameStateNode::qualityEvaluator() const
    {
        return *mEvaluator;
    }


    const GameState & GameStateNode::state() const
    {
        return *mGameState;
    }


    GameState & GameStateNode::state()
    {
        return *mGameState;
    }


    const ChildNodes & GameStateNode::children() const
    {
        return mChildren;
    }


    void GameStateNode::clearChildren()
    {
        mChildren.clear();
    }


    void GameStateNode::addChild(NodePtr inChildNode)
    {
        Assert(inChildNode->depth() == mDepth + 1);
        mChildren.insert(inChildNode);
    }


    const GameStateNode * GameStateNode::endNode() const
    {
        if (mChildren.empty())
        {
            return this;
        }
        return (*mChildren.begin())->endNode();
    }


    GameStateNode * GameStateNode::endNode()
    {
        if (mChildren.empty())
        {
            return this;
        }
        return (*mChildren.begin())->endNode();
    }


    NodePtr GameStateNode::parent()
    {
        return mParent.lock();
    }


    int GameStateNode::depth() const
    {
        return mDepth;
    }


    const NodePtr GameStateNode::parent() const
    {
        return mParent.lock();
    }


    GameStateComparisonFunctor::GameStateComparisonFunctor(std::auto_ptr<Evaluator> inEvaluator) :
        mEvaluator(inEvaluator.release())
    {
    }


    bool GameStateComparisonFunctor::operator()(NodePtr lhs, NodePtr rhs)
    {
        Assert(lhs && rhs);
        Assert(lhs.get() != rhs.get());

        // Order by descending quality.
        return lhs->state().quality(*mEvaluator) > rhs->state().quality(*mEvaluator);
    }

} // namespace Tetris
