#include "GameStateNode.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"

namespace Tetris
{


    std::auto_ptr<GameStateNode> GameStateNode::CreateRootNode(size_t inNumRows, size_t inNumColumns)
    {
        return std::auto_ptr<GameStateNode>(new GameStateNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))));
    }


    GameStateNode::GameStateNode(std::auto_ptr<GameState> inGameState) :
        mParent(0),
        mDepth(0),
        mGameState(inGameState)
    {

    }


    GameStateNode::GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState) :
        mParent(inParent),
        mDepth(inParent->depth() + 1),
        mGameState(inGameState)
    {
    }


    void CollectAll(GameStateNode * inChild, int inDepth, ChildNodes & outChildren)
    {
        if (inDepth > 1)
        {
            ChildNodes children = inChild->children();
            for (ChildNodes::iterator it = children.begin(); it != children.end(); ++it)
            {
                CollectAll(it->get(), inDepth - 1, outChildren);
            }
        }
        else
        {
            ChildNodes children = inChild->children();
            for (ChildNodes::iterator it = children.begin(); it != children.end(); ++it)
            {
                outChildren.insert(*it);
            }
        }
    }


    // Collect all children at the requested depth from all branches,
    // and return the one with the highest quality score.
    GameStateNode * GameStateNode::bestChild(int inDepth)
    {
        ChildNodePtr result;
        ChildNodes collectAll;
        CollectAll(this, inDepth, collectAll);
        if (!collectAll.empty())
        {
            result = *collectAll.begin();
        }
        return result.get();
    }


    const GameState & GameStateNode::state() const
    {
        return *mGameState;
    }


    GameState & GameStateNode::state()
    {
        return *mGameState;
    }


    ChildNodes & GameStateNode::children()
    {
        return mChildren;
    }


    const ChildNodes & GameStateNode::children() const
    {
        return mChildren;
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
        const GameState & gameState = ioGameStateNode.state();
        const Grid & gameGrid = gameState.grid();

        // Is this a "game over" situation?
        // If yes then append the final "broken" game state as only child.
        if (IsGameOver(gameState, inBlockType, 0))
        {
            size_t initialColumn = DivideByTwo(gameGrid.numColumns() - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns());
            ChildNodePtr childState(new GameStateNode(&ioGameStateNode, gameState.commit(Block(inBlockType, Rotation(0), Row(0), Column(initialColumn)), true)));
            outChildNodes.insert(childState);
            return;
        }

        for (size_t col = 0; col != gameGrid.numColumns(); ++col)
        {
            Block block(inBlockType, Rotation(0), Row(0), Column(col));
            for (size_t rt = 0; rt != block.numRotations(); ++rt)
            {
                std::auto_ptr<GameState> newGameState;
                block.setRotation(rt);
                size_t row = 0;
                while (gameState.checkPositionValid(block, row, col))
                {
                    block.setRow(row);
                    row++;
                }
                ChildNodePtr childState(new GameStateNode(&ioGameStateNode, gameState.commit(block, false)));
                outChildNodes.insert(childState);
            }
        }
    }


    void GenerateOffspring(BlockTypes inBlockTypes, size_t inOffset, GameStateNode & ioGameStateNode, ChildNodes & outChildNodes)
    {
        CheckPrecondition(inOffset < inBlockTypes.size(), "GenerateOffspring: inOffset must be smaller than inBlockTypes.size().");
        
        GenerateOffspring(inBlockTypes[inOffset], ioGameStateNode, outChildNodes);

        if (inOffset + 1 < inBlockTypes.size())
        {
            ChildNodes::iterator it = outChildNodes.begin(), end = outChildNodes.end();
            for (; it != end; ++it)
            {
                ChildNodePtr childNode = *it;
                GenerateOffspring(inBlockTypes, inOffset + 1, *childNode, childNode->children());
            }
        }
    }


    bool ChildPtrCompare::operator()(ChildNodePtr lhs, ChildNodePtr rhs)
    {
        CheckArgument(lhs && rhs, "Comparison fails because ChildNodePtr objects are null!");
        CheckArgument(lhs.get() != rhs.get(), "Comparison game state against itself. This is wrong!");

        return lhs->state().quality() > rhs->state().quality();
    }

} // namespace Tetris
