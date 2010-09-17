#include "Tetris/GameStateNode.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/Block.h"
#include "Tetris/ErrorHandling.h"
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
        std::auto_ptr<GameStateNode> result(parent ? new GameStateNode(parent,
                                                                       std::auto_ptr<GameState>(new GameState(*mGameState)),
                                                                       mEvaluator->clone())
                                                    : new GameStateNode(std::auto_ptr<GameState>(new GameState(*mGameState)),
                                                                        mEvaluator->clone()));
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


    bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation)
    {
        const Grid & blockGrid = GetGrid(GetBlockIdentifier(inBlockType, inRotation));
        const Grid & gameGrid = inGameState.grid();
        size_t initialColumn = DivideByTwo(inGameState.grid().numColumns() - blockGrid.numColumns());
        for (size_t row = 0; row < blockGrid.numRows(); ++row)
        {
            for (size_t col = 0; col < blockGrid.numColumns(); ++col)
            {
                if (blockGrid.get(row, col) != BlockType_Nil && gameGrid.get(row, initialColumn + col) != BlockType_Nil)
                {
                    return true;
                }
            }
        }
        return false;
    }


    void GenerateOffspring(NodePtr inNode,
                           BlockType inBlockType,
                           const Evaluator & inEvaluator,
                           ChildNodes & outChildNodes)
    {        
        Assert(outChildNodes.empty());
        const GameState & gameState = inNode->state();
        const Grid & gameGrid = gameState.grid();

        // Is this a "game over" situation?
        // If yes then append the final "broken" game state as only child.
        if (IsGameOver(gameState, inBlockType, 0))
        {
            size_t initialColumn = DivideByTwo(gameGrid.numColumns() - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns());
            NodePtr childState(new GameStateNode(inNode,
                                                 gameState.commit(Block(inBlockType,
                                                                        Rotation(0),
                                                                        Row(0),
                                                                        Column(initialColumn)),
                                                                        GameOver(true)),
                                                 inEvaluator.clone()));
            Assert(childState->depth() == (inNode->depth() + 1));
            outChildNodes.insert(childState);
            return;
        }

        for (size_t col = 0; col != gameGrid.numColumns(); ++col)
        {
            Block block(inBlockType, Rotation(0), Row(0), Column(col));
            for (size_t rt = 0; rt != block.numRotations(); ++rt)
            {
                block.setRotation(rt);
                size_t row = 0;
                while (gameState.checkPositionValid(block, row, col))
                {
                    row++;
                }
                if (row > 0)
                {
                    block.setRow(row - 1);
                    NodePtr childState(new GameStateNode(inNode,
                                                         gameState.commit(block, GameOver(false)),
                                                         inEvaluator.clone()));
                    Assert(childState->depth() == inNode->depth() + 1);
                    outChildNodes.insert(childState);
                }
            }
        }
    }


    void GenerateOffspring(NodePtr ioGameStateNode,
                           BlockTypes inBlockTypes,
                           size_t inOffset,
                           const Evaluator & inEvaluator)
    {
        Assert(inOffset < inBlockTypes.size());
        Assert(ioGameStateNode->children().empty());

        GenerateOffspring(ioGameStateNode,
                          inBlockTypes[inOffset],                          
                          inEvaluator,
                          ioGameStateNode->children());

        if (inOffset + 1 < inBlockTypes.size())
        {
            for (ChildNodes::iterator it = ioGameStateNode->children().begin();
                 it != ioGameStateNode->children().end();
                 ++it)
            {
                NodePtr childNode = *it;
                GenerateOffspring(childNode,
                                  inBlockTypes,
                                  inOffset + 1,
                                  inEvaluator);
            }
        }
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
