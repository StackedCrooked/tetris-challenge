#include "Tetris/AISupport.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/Grid.h"
#include "Tetris/Utilities.h"
#include "Futile/Assert.h"


namespace Tetris {


void CarveBestPath(NodePtr startNode, NodePtr dst)
{
    Assert(dst->depth() > startNode->depth());
    while (dst->depth() != startNode->depth())
    {
        ChildNodes::const_iterator it = dst->parent()->children().begin(), end = dst->parent()->children().end();
        for (; it != end; ++it)
        {
            NodePtr endNode = *it;
            if (endNode == dst) // is endNode part of the path between startNode and endNode?
            {
                // Erase all children. The endNode object is kept alive by endNode object.
                dst->parent()->clearChildren();

                // Add endNode to the child nodes again.
                dst->parent()->addChild(endNode);
                break;
            }
        }
        dst = dst->parent();
    }
}


bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation)
{
    const Grid & blockGrid = GetGrid(GetBlockIdentifier(inBlockType, inRotation));
    const Grid & gameGrid = inGameState.grid();
    std::size_t initialColumn = DivideByTwo(inGameState.grid().columnCount() - blockGrid.columnCount());
    for (std::size_t row = 0; row < blockGrid.rowCount(); ++row)
    {
        for (std::size_t col = 0; col < blockGrid.columnCount(); ++col)
        {
            if (blockGrid.get(row, col) != BlockType_Nil && gameGrid.get(row, initialColumn + col) != BlockType_Nil)
            {
                return true;
            }
        }
    }
    return false;
}


void GenerateOffspring(NodePtr ioGameStateNode,
                       BlockTypes inBlockTypes,
                       std::size_t inOffset,
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


void GenerateOffspring(NodePtr inNode,
                       BlockType inBlockType,
                       const Evaluator & inEvaluator,
                       ChildNodes & outChildNodes)
{
    Assert(outChildNodes.empty());
    const GameState & gameState = inNode->gameState();
    const Grid & gameGrid = gameState.grid();

    // Is this a "game over" situation?
    // If yes then append the final "broken" game state as only child.
    if (IsGameOver(gameState, inBlockType, 0))
    {
        std::size_t initialColumn = DivideByTwo(gameGrid.columnCount() - GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount());
        NodePtr childState(new GameStateNode(inNode,
                                             gameState.commit(Block(inBlockType, Rotation(0), Row(0), Column(initialColumn)),
                                                              GameOver(true)),
                                             inEvaluator));
        Assert(childState->depth() == (inNode->depth() + 1));
        outChildNodes.insert(childState);
        return;
    }

    // Generate game state for each column/rotation combination.
    for (std::size_t col = 0; col != gameGrid.columnCount(); ++col)
    {
        Block block(inBlockType, Rotation(0), Row(0), Column(col));
        for (std::size_t rt = 0; rt != block.rotationCount(); ++rt)
        {
            block.setRotation(rt);
            std::size_t row = 0;
            while (gameState.checkPositionValid(block, row, col))
            {
                row++;
            }
            if (row > 0)
            {
                block.setRow(row - 1);
                NodePtr childState(new GameStateNode(inNode,
                                                     gameState.commit(block, GameOver(false)),
                                                     inEvaluator));
                Assert(childState->depth() == inNode->depth() + 1);
                outChildNodes.insert(childState);
            }
        }
    }
}


} // namespace Tetris
