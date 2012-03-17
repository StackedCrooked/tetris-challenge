#ifndef TETRIS_AISUPPORT_H
#define TETRIS_AISUPPORT_H


#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"
#include <boost/function.hpp>
#include <ostream>


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
void CarveBestPath(const NodePtr & startNode, NodePtr & endNode);

bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation);

void GenerateOffspring(const NodePtr & ioGameStateNode,
                       const BlockTypes & inBlockTypes,
                       std::size_t inOffset,
                       const Evaluator & inEvaluator);

void GenerateOffspring(const NodePtr & ioGameStateNode,
                       BlockType inBlockType,
                       const Evaluator & inEvaluator,
                       ChildNodes & outChildNodes);

class Progress
{
public:
    explicit Progress(unsigned inMaxValue) :
        mCurrent(0),
        mMaximum(inMaxValue)
    {
    }

    Progress(unsigned inCurValue, unsigned inMaxValue) :
        mCurrent(inCurValue),
        mMaximum(inMaxValue)
    {
    }

    unsigned current() const { return mCurrent; }

    unsigned limit() const { return mMaximum; }

    bool complete() const { return mCurrent == mMaximum; }

    Progress increment(unsigned amount = 1) const
    {
        return Progress(std::min<unsigned>(current() + amount, limit()), limit());
    }

private:
    unsigned mCurrent;
    unsigned mMaximum;
};


inline std::ostream & operator<<(std::ostream & os, const Progress & progress)
{
    os << progress.current() << "/" << progress.limit();
	return os;
}


typedef boost::function<void(const Progress &, const NodePtr &)> ChildNodeGeneratedCallback;


void CalculateNodes(const NodePtr & ioNode,
                    const Evaluator & inEvaluator,
                    const BlockTypes & inBlockTypes,
                    const std::vector<int> & inWidths,
                    const Progress & inProgress,
                    const ChildNodeGeneratedCallback & inCallback = ChildNodeGeneratedCallback(),
                    unsigned _checkChildCount = 0); // internal checking mechanism


} // namespace Tetris


#endif // TETRIS_AISUPPORT_H

