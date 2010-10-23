#ifndef TETRIS_MULTITHREADEDNODECALCULATORIMPL_H_INCLUDED
#define TETRIS_MULTITHREADEDNODECALCULATORIMPL_H_INCLUDED


#include "Tetris/NodeCalculatorImpl.h"


namespace Tetris
{

    class MultithreadedNodeCalculator : public NodeCalculatorImpl
    {
    public:
        MultithreadedNodeCalculator(std::auto_ptr<GameStateNode> inNode,
                                    const BlockTypes & inBlockTypes,
                                    const std::vector<int> & inWidths,
                                    std::auto_ptr<Evaluator> inEvaluator,
                                    WorkerPool & inWorkerPool);

        virtual ~MultithreadedNodeCalculator();

    private:
        virtual void populate();
        
        void populateNodes(NodePtr ioNode,
                           const BlockTypes & inBlockTypes,
                           const std::vector<int> & inWidths,
                           size_t inIndex,
                           size_t inMaxIndex);
        
        void populateNodesMt(NodePtr ioNode,
                             const BlockTypes & inBlockTypes,
                             const std::vector<int> & inWidths,
                             size_t inIndex,
                             size_t inMaxIndex);
    };

} // namespace Tetris


#endif // TETRIS_MULTITHREADEDNODECALCULATORIMPL_H_INCLUDED

