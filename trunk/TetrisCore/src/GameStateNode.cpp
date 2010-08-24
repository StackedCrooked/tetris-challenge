#include "GameStateNode.h"
#include "GameQualityEvaluator.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"

namespace Tetris
{


    std::auto_ptr<GameStateNode> GameStateNode::CreateRootNode(size_t inNumRows, size_t inNumColumns)
    {
        return std::auto_ptr<GameStateNode>(
            new GameStateNode(
                std::auto_ptr<GameState>(
                    new GameState(new DefaultGameQualityEvaluator, inNumRows, inNumColumns))));
    }


    static int GetIdentifier(const GameState & inGameState)
    {
        const Block & block = inGameState.originalBlock();
        return block.numRotations() * block.column() + block.rotation();
    }


    GameStateNode::GameStateNode(std::auto_ptr<GameState> inGameState) :
        mParent(0),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(0),
        mGameState(inGameState)
    {

    }


    GameStateNode::GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState) :
        mParent(inParent),
        mIdentifier(GetIdentifier(*inGameState)),
        mDepth(inParent->depth() + 1),
        mGameState(inGameState)
    {
    }


    std::auto_ptr<GameStateNode> GameStateNode::clone() const
    {
        std::auto_ptr<GameStateNode> result(mParent ? new GameStateNode(mParent, std::auto_ptr<GameState>(new GameState(*mGameState)))
                                                    : new GameStateNode(std::auto_ptr<GameState>(new GameState(*mGameState))));
        result->mDepth = mDepth;

        ChildNodes::const_iterator it = mChildren.begin(), end = mChildren.end();
        for (; it != end; ++it)
        {
            GameStateNode & node(*(*it));
            ChildNodePtr newChild(node.clone().release());
            Assert(newChild->depth() == mDepth + 1);
            result->mChildren.insert(newChild);
        }
        return result;
    }


    int GameStateNode::identifier() const
    {
        return mIdentifier;
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


    void GameStateNode::addChild(ChildNodePtr inChildNode)
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


    GameStateNode * GameStateNode::parent()
    {
        return mParent;
    }


    int GameStateNode::depth() const
    {
        return mDepth;
    }


    const GameStateNode * GameStateNode::parent() const
    {
        return mParent;
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


    void GenerateOffspring(BlockType inBlockType, GameStateNode & ioGameStateNode, ChildNodes & outChildNodes)
    {        
        Assert(outChildNodes.empty());
        const GameState & gameState = ioGameStateNode.state();
        const Grid & gameGrid = gameState.grid();

        // Is this a "game over" situation?
        // If yes then append the final "broken" game state as only child.
        if (IsGameOver(gameState, inBlockType, 0))
        {
            size_t initialColumn = DivideByTwo(gameGrid.numColumns() - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns());
            ChildNodePtr childState(new GameStateNode(&ioGameStateNode,
                                                      gameState.commit(Block(inBlockType,
                                                                             Rotation(0),
                                                                             Row(0),
                                                                             Column(initialColumn)),
                                                                             GameOver(true))));
            Assert(childState->depth() == (ioGameStateNode.depth() + 1));
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
                    ChildNodePtr childState(new GameStateNode(&ioGameStateNode, gameState.commit(block, GameOver(false))));
                    Assert(childState->depth() == ioGameStateNode.depth() + 1);
                    outChildNodes.insert(childState);
                }
            }
        }
    }


    void GameStateNode::generateOffspring(BlockTypes inBlockTypes, size_t inOffset)
    {
        Assert(inOffset < inBlockTypes.size());

        Assert(mChildren.empty());
        GenerateOffspring(inBlockTypes[inOffset], *this, mChildren);

        if (inOffset + 1 < inBlockTypes.size())
        {
            ChildNodes::iterator it = mChildren.begin(), end = mChildren.end();
            for (; it != end; ++it)
            {
                GameStateNode & childNode = **it;
                childNode.generateOffspring(inBlockTypes, inOffset + 1);
            }
        }
    }


    bool ChildPtrCompare::operator()(ChildNodePtr lhs, ChildNodePtr rhs)
    {
        Assert(lhs && rhs);
        Assert(lhs.get() != rhs.get());
        return lhs->state().quality() > rhs->state().quality();
    }

} // namespace Tetris
