#ifndef TETRIS_AISUPPORT_H_INCLUDED
#define TETRIS_AISUPPORT_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"


namespace Tetris {


class Evaluator;
class GameState;


/**
 * CarveBestPath
 *
 * Removes all tree-nodes that are not in the best path.
 *
 * The best path is defined as the path that is formed by backtracking the
 * ancestry (parent nodes) of endNode's first child up until the start node.
 */
void CarveBestPath(NodePtr startNode, NodePtr endNode);

bool IsGameOver(const GameState& inGameState, BlockType inBlockType, int inRotation);

void GenerateOffspring(NodePtr ioGameStateNode,
                       BlockTypes inBlockTypes,
                       std::size_t inOffset,
                       const Evaluator& inEvaluator);

void GenerateOffspring(NodePtr ioGameStateNode,
                       BlockType inBlockType,
                       const Evaluator& inEvaluator,
                       ChildNodes& outChildNodes);


} // namespace Tetris


#endif // AISUPPORT_H_INCLUDED

