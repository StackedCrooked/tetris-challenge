#ifndef TETRIS_SINGLETHREADEDNODECALCULATORIMPL_H_INCLUDED
#define TETRIS_SINGLETHREADEDNODECALCULATORIMPL_H_INCLUDED


#include "Tetris/NodeCalculatorImpl.h"


namespace Tetris
{

    class SingleThreadedNodeCalculator : public NodeCalculatorImpl
    {
    public:
        SingleThreadedNodeCalculator(std::auto_ptr<GameStateNode> inNode,
                                     const BlockTypes & inBlockTypes,
                                     const std::vector<int> & inWidths,
                                     std::auto_ptr<Evaluator> inEvaluator,
                                     WorkerPool & inWorkerPool);

        virtual ~SingleThreadedNodeCalculator() {}

    private:
        virtual void populate();
    };

} // namespace Tetris


#endif // TETRIS_SINGLETHREADEDNODECALCULATORIMPL_H_INCLUDED

