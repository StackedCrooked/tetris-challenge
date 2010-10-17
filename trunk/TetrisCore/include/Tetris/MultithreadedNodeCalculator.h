#ifndef TETRIS_MULTITHREADEDNODECALCULATOR_H_INCLUDED
#define TETRIS_MULTITHREADEDNODECALCULATOR_H_INCLUDED


#include "Tetris/AbstractNodeCalculator.h"
#include "Tetris/BlockTypes.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <memory>


namespace Tetris
{

    class Evaluator;
    class GameStateNode;
    class WorkerPool;
    typedef std::vector<int> Widths;


    class MultithreadedNodeCalculatorImpl;


    class MultithreadedNodeCalculator : public AbstractNodeCalculator
    {
    public:
        MultithreadedNodeCalculator(boost::shared_ptr<WorkerPool> inWorkerPool,
                                    std::auto_ptr<GameStateNode> inNode,
                                    const BlockTypes & inBlockTypes,
                                    const Widths & inWidths,
                                    std::auto_ptr<Evaluator> inEvaluator);

        virtual ~MultithreadedNodeCalculator();

        virtual void start();

        virtual void stop();

        virtual int getCurrentSearchDepth() const;

        virtual int getMaxSearchDepth() const;

        virtual NodePtr result() const;

        AbstractNodeCalculator::Status status() const;

    private:
        MultithreadedNodeCalculator(const MultithreadedNodeCalculator &);
        MultithreadedNodeCalculator & operator=(const MultithreadedNodeCalculator &);

        MultithreadedNodeCalculatorImpl * mImpl;
    };

} // namespace Tetris


#endif // TETRIS_MULTITHREADEDNODECALCULATOR_H_INCLUDED
