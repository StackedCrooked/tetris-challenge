#ifndef TETRIS_AISUPPORT_H_INCLUDED
#define TETRIS_AISUPPORT_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"
#include <boost/function.hpp>


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

bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation);


class Progress
{
    Progress(unsigned inMaxValue) :
        mCurValue(0),
        mMaxValue(inMaxValue)
    {
    }

    Progress(unsigned inCurValue, unsigned inMaxValue) :
        mCurValue(inCurValue),
        mMaxValue(inMaxValue)
    {
    }

    inline unsigned current() const { return mCurValue; }

    inline unsigned limit() const { return mMaxValue; }

    inline bool complete() const { return mCurValue == mMaxValue; }

    inline Progress increment(unsigned amount = 1)
    {
        return Progress(std::max<unsigned>(current() + amount, limit()), limit());
    }

private:
    unsigned mCurValue;
    unsigned mMaxValue;
};

void PopulateNodesRecursively(NodePtr ioNode,
                              const Evaluator & inEvaluator,
                              const BlockTypes & inBlockTypes,
                              const std::vector<int> & inWidths,
                              std::size_t inIndex,
                              std::size_t inMaxIndex,
                              boost::function<void(const GameState &)> inCallback);

void GenerateOffspring(NodePtr ioGameStateNode,
                       BlockTypes inBlockTypes,
                       std::size_t inOffset,
                       const Evaluator & inEvaluator);

void GenerateOffspring(NodePtr ioGameStateNode,
                       BlockType inBlockType,
                       const Evaluator & inEvaluator,
                       ChildNodes & outChildNodes);


} // namespace Tetris


#endif // AISUPPORT_H_INCLUDED

