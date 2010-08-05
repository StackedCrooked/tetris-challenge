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


    void GenerateOffspring(const Block & inBlock, GameStateNode & ioGameStateNode, ChildNodes & outChildNodes)
    {
        CheckPrecondition(outChildNodes.empty(), "GenerateOffspring: outChildNodes already has children.");
        
        const GameState & gameState = ioGameStateNode.state();
        const Grid & gameGrid = gameState.grid();

        if (IsGameOver(gameState, inBlock.type(), inBlock.rotation()))
        {
            return;
        }

        for (size_t col = 0; col != gameGrid.numColumns(); ++col)
        {
            Block block = inBlock;
            block.setColumn(col);
            block.setRow(0);
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

                if (row > 0)
                {
                    newGameState = gameState.commit(block, false);
                }
                else
                {
                    newGameState = gameState.commit(block, true);
                }
                ChildNodePtr childState(new GameStateNode(&ioGameStateNode, newGameState));
                outChildNodes.insert(childState);
            }
        }
    }


    void GameStateNode::populate(const Block & inBlock)
    {
        GenerateOffspring(inBlock, *this, mChildren);
    }


    bool ChildPtrCompare::operator()(ChildNodePtr lhs, ChildNodePtr rhs)
    {
        CheckArgument(lhs && rhs, "Comparison fails because ChildNodePtr objects are null!");
        CheckArgument(lhs.get() != rhs.get(), "Comparison game state against itself. This is wrong!");

        return lhs->state().quality() > rhs->state().quality();
    }

} // namespace Tetris
