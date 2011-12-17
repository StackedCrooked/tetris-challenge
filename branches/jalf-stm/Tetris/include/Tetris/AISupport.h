#ifndef TETRIS_AISUPPORT_H
#define TETRIS_AISUPPORT_H


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
public:
    Progress(unsigned inMaxValue) :
        mCurrent(0),
        mMaximum(inMaxValue)
    {
    }

    Progress(unsigned inCurValue, unsigned inMaxValue) :
        mCurrent(inCurValue),
        mMaximum(inMaxValue)
    {
    }

    inline unsigned current() const { return mCurrent; }

    inline unsigned limit() const { return mMaximum; }

    inline bool complete() const { return mCurrent == mMaximum; }

    inline Progress increment(unsigned amount = 1) const
    {
        return Progress(std::max<unsigned>(current() + amount, limit()), limit());
    }

private:
    unsigned mCurrent;
    unsigned mMaximum;
};


void CalculateNodes(const GameState & inGameState,
                    const Evaluator & inEvaluator,
                    const BlockTypes & inBlockTypes,
                    const std::vector<int> & inWidths,
                    const Progress & inProgress,
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


#endif // TETRIS_AISUPPORT_H

