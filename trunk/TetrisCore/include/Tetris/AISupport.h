#ifndef AISUPPORT_H_INCLUDED
#define AISUPPORT_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/GameStateNode.h"


namespace Tetris
{

    // Remove all the children from srcNode except the one on the path that leads to dstNode.
    // The children of the 'good one' are also exterminated, except the one that brings us a 
    // step closer to dstNode. This algorithm is recursively repeated until only the path between
    // srcNode and dstNode remains.
    //
    // The purpose of this function is mainly to free up memory.
    //
    void DestroyInferiorChildren(NodePtr startNode, NodePtr endNode);

    bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation);
    
    void GenerateOffspring(NodePtr ioGameStateNode,
                           BlockTypes inBlockTypes,
                           size_t inOffset,
                           const Evaluator & inEvaluator);

    void GenerateOffspring(NodePtr ioGameStateNode,
                           BlockType inBlockType,
                           const Evaluator & inEvaluator,
                           ChildNodes & outChildNodes);

} // namespace Tetris


#endif // AISUPPORT_H_INCLUDED
