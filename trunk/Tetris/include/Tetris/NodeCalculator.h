#ifndef TETRIS_NODECALCULATOR_H_INCLUDED
#define TETRIS_NODECALCULATOR_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"
#include "Futile/WorkerPool.h"
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <memory>


namespace Tetris {


class Evaluator;
class GameStateNode;
class NodeCalculatorImpl;


class NodeCalculator
{
public:
    NodeCalculator(std::auto_ptr<GameStateNode> inNode,
                   const BlockTypes & inBlockTypes,
                   const std::vector<int> & inWidths,
                   const Evaluator & inEvaluator,
                   Futile::Worker & inMainWorker,
                   Futile::WorkerPool & inWorkerPool);

    ~NodeCalculator();

    void start();

    void stop();

    unsigned getNumberOfCalculatedNodes() const;

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
        Status_Error,
        Status_End
    };

    Status status() const;

    // Returns the error message (in case of Status_Error).
    std::string errorMessage() const;

private:
    // non-copyable
    NodeCalculator(const NodeCalculator &);
    NodeCalculator & operator=(const NodeCalculator &);

    boost::scoped_ptr<NodeCalculatorImpl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATOR_H_INCLUDED
