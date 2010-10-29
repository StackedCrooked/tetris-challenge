#ifndef TETRIS_NODECALCULATOR_H_INCLUDED
#define TETRIS_NODECALCULATOR_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"
#include <vector>
#include <memory>


namespace Tetris
{

    class Evaluator;
    class GameStateNode;
    class WorkerPool;
    class NodeCalculatorImpl;


    class NodeCalculator
    {
    public:
        NodeCalculator(std::auto_ptr<GameStateNode> inNode,
                       const BlockTypes & inBlockTypes,
                       const std::vector<int> & inWidths,
                       std::auto_ptr<Evaluator> inEvaluator,
                       WorkerPool & inWorkerPool);

        ~NodeCalculator();

        void start();

        void stop();

        int getCurrentSearchDepth() const;

        int getMaxSearchDepth() const;

        NodePtr result() const;

        enum Status
        {
            Status_Nil,
            Status_Begin,
            Status_Started = Status_Begin,
            Status_Working,
            Status_Stopped,
            Status_Finished,
            Status_End
        };

        Status status() const;

    private:
        // non-copyable
        NodeCalculator(const NodeCalculator &);
        NodeCalculator & operator=(const NodeCalculator &);

        NodeCalculatorImpl * mImpl;
    };

} // namespace Tetris


#endif // TETRIS_NODECALCULATOR_H_INCLUDED

