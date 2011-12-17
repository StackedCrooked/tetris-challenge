#ifndef TETRIS_MULTITHREADEDNODECALCULATORIMPL_H
#define TETRIS_MULTITHREADEDNODECALCULATORIMPL_H


#include "Tetris/NodeCalculatorImpl.h"
#include "boost/shared_ptr.hpp"


namespace Tetris {


class MultithreadedNodeCalculator : public NodeCalculatorImpl
{
public:
    MultithreadedNodeCalculator(const GameState & inGameState,
                                const BlockTypes & inBlockTypes,
                                const std::vector<int> & inWidths,
                                const Evaluator & inEvaluator,
                                Futile::Worker & inMainWorker,
                                Futile::WorkerPool & inWorkerPool);

    virtual ~MultithreadedNodeCalculator();

private:
    virtual void populate();

    void generateChildNodes(NodePtr ioNode,
                            const Evaluator * inEvaluator,
                            BlockType inBlockType,
                            int inWidth);

    void populateNodes(NodePtr ioNode,
                       const BlockTypes & inBlockTypes,
                       const std::vector<int> & inWidths,
                       std::size_t inIndex,
                       std::size_t inEndIndex);
};


} // namespace Tetris


#endif // TETRIS_MULTITHREADEDNODECALCULATORIMPL_H
