#ifndef TETRIS_NODECALCULATOR_H_INCLUDED
#define TETRIS_NODECALCULATOR_H_INCLUDED


#include "Tetris/AbstractNodeCalculator.h"
#include "Tetris/BlockTypes.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <memory>


namespace Tetris
{

    class Evaluator;
    class Worker;
    typedef std::vector<int> Widths;


    class NodeCalculatorImpl;


    class NodeCalculator : public AbstractNodeCalculator
    {
    public:
        NodeCalculator(boost::shared_ptr<Worker> inWorker,
                       std::auto_ptr<GameStateNode> inNode,
                       const BlockTypes & inBlockTypes,
                       const Widths & inWidths,
                       std::auto_ptr<Evaluator> inEvaluator);

        virtual ~NodeCalculator();

        virtual void start();

        virtual void stop();

        virtual int getCurrentSearchDepth() const;

        virtual int getMaxSearchDepth() const;

        virtual NodePtr result() const;

        Status status() const;

    private:
        NodeCalculator(const NodeCalculator &);
        NodeCalculator & operator=(const NodeCalculator &);

        NodeCalculatorImpl * mImpl;
    };

} // namespace Tetris


#endif // TETRIS_NODECALCULATOR_H_INCLUDED

