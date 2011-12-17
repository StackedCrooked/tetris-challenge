#ifndef TETRIS_SINGLETHREADEDNODECALCULATORIMPL_H
#define TETRIS_SINGLETHREADEDNODECALCULATORIMPL_H


#include "Tetris/NodeCalculatorImpl.h"


namespace Tetris {


class SingleThreadedNodeCalculator : public NodeCalculatorImpl
{
public:
    SingleThreadedNodeCalculator(const GameState & inGameState,
                                 const BlockTypes & inBlockTypes,
                                 const std::vector<int> & inWidths,
                                 const Evaluator & inEvaluator,
                                 Futile::Worker & inMainWorker,
                                 Futile::WorkerPool & inWorkerPool);

    virtual ~SingleThreadedNodeCalculator();

private:
    virtual void populate();
};


} // namespace Tetris


#endif // TETRIS_SINGLETHREADEDNODECALCULATORIMPL_H
