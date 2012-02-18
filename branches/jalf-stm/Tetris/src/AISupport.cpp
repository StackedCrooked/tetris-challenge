#include "Tetris/AISupport.h"
#include "Futile/Assert.h"
#include "Tetris/Block.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Grid.h"
#include "Tetris/Utilities.h"
#include "Futile/Logging.h"
#include <algorithm>


namespace Tetris {


void CarveBestPath(const NodePtr & startNode, NodePtr & dst)
{
    Assert(dst->depth() > startNode->depth());
    while (dst->depth() != startNode->depth())
    {
        ChildNodes::const_iterator it = dst->parent()->children().begin(), end = dst->parent()->children().end();
        for (; it != end; ++it)
        {
            const NodePtr & endNode = *it;
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
    std::size_t initialColumn = InitialBlockPosition(inGameState.grid().columnCount(), blockGrid.columnCount());
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


void CalculateNodes(const NodePtr & ioNode,
                    unsigned & outTotalNodeCount,
                    const Evaluator & inEvaluator,
                    const BlockTypes & inBlockTypes,
                    const std::vector<int> & inWidths,
                    const Progress & inProgress,
                    const ChildNodeGeneratedCallback & inCallback,
                    unsigned _checkChildCount)
{
    // We want to at least perform a search of depth 4.
    if (inProgress.current() >= 4)
    {
        boost::this_thread::interruption_point();
    }


    //
    // Check stop conditions
    //
    if (inProgress.complete() || inProgress.current() >= inBlockTypes.size())
    {
        Assert(_checkChildCount  > 0);
        return;
    }


    if (ioNode->gameState().isGameOver())
    {
        // GameOver state has no children.
        Assert(_checkChildCount  > 0);
        return;
    }


    //
    // Generate the child nodes.
    //
    // It is possible that the nodes were already generated at this depth.
    // If that is the case then we immediately jump to the recursive call below.
    //
    ChildNodes generatedChildNodes = ioNode->children();

    int numNewChildren = 0;
    if (generatedChildNodes.empty())
    {
        generatedChildNodes = ChildNodes(GameStateComparator());
        GenerateOffspring(ioNode, inBlockTypes[inProgress.current()], inEvaluator, generatedChildNodes, outTotalNodeCount);

        ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
        while (numNewChildren < inWidths[inProgress.current()] && it != end)
        {
            ioNode->addChild(*it);
            ++numNewChildren;
            _checkChildCount = numNewChildren;
            ++it;
        }

        Assert(numNewChildren >= 1);
        const NodePtr & gameStateNode = *ioNode->children().begin();

        if (inCallback)
        {
            inCallback(inProgress, gameStateNode);
        }
    }

    //
    // Recursive call on each child node.
    //
    for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
    {
        const NodePtr & child = *it;
        CalculateNodes(child, outTotalNodeCount, inEvaluator, inBlockTypes, inWidths, inProgress.increment(), inCallback, _checkChildCount + numNewChildren);
    }
}


void GenerateOffspring(const NodePtr & ioGameStateNode,
                       const BlockTypes & inBlockTypes,
                       std::size_t inOffset,
                       const Evaluator & inEvaluator,
                       unsigned & outTotalCounter)
{
    Assert(inOffset < inBlockTypes.size());
    Assert(ioGameStateNode->children().empty());

    ChildNodes children;
    GenerateOffspring(ioGameStateNode,
                      inBlockTypes[inOffset],
                      inEvaluator,
                      children,
                      outTotalCounter);

    std::for_each(children.begin(), children.end(), [&](const NodePtr & child) { ioGameStateNode->addChild(child); });

    if (inOffset + 1 < inBlockTypes.size())
    {
        for (ChildNodes::const_iterator it = ioGameStateNode->children().begin();
                it != ioGameStateNode->children().end();
                ++it)
        {
            const NodePtr & childNode = *it;
            GenerateOffspring(childNode,
                              inBlockTypes,
                              inOffset + 1,
                              inEvaluator,
                              outTotalCounter);
        }
    }
}


void GenerateOffspring(const NodePtr & inNode,
                       BlockType inBlockType,
                       const Evaluator & inEvaluator,
                       ChildNodes & outChildNodes,
                       unsigned & outTotalCounter)
{
    Assert(outChildNodes.empty());
    const GameState & gameState = inNode->gameState();
    const Grid & gameGrid = gameState.grid();

    // Is this a "game over" situation?
    // If yes then append the final "broken" game state as only child.
    if (IsGameOver(gameState, inBlockType, 0))
    {
        std::size_t initialColumn = InitialBlockPosition(gameGrid.columnCount(),
                                                         GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount());
        NodePtr childState(new GameStateNode(inNode,
                                             gameState.commit(Block(inBlockType,
                                                                    Rotation(0),
                                                                    Row(0),
                                                                    Column(initialColumn))),
                                             inEvaluator));
        Assert(childState->gameState().isGameOver());
        Assert(childState->depth() == (inNode->depth() + 1));
        outChildNodes.insert(childState);
        outTotalCounter++;
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
                                                     gameState.commit(block),
                                                     inEvaluator));
                Assert(childState->depth() == inNode->depth() + 1);
                outChildNodes.insert(childState);
                outTotalCounter++;
            }
        }
    }
}


} // namespace Tetris
