#ifndef TETRIS_NODECALCULATOR_H
#define TETRIS_NODECALCULATOR_H


#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"
#include "Futile/WorkerPool.h"
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <memory>
#include <vector>


namespace Tetris {


class Evaluator;
class GameStateNode;
class NodeCalculatorImpl;


class NodeCalculator : boost::noncopyable
{
public:
    NodeCalculator(const GameState & inGameState,
                   const BlockTypes & inBlockTypes,
                   const std::vector<int> & inWidths,
                   const Evaluator & inEvaluator,
                   Futile::Worker & inMainWorker,
                   Futile::WorkerPool & inWorkerPool);

    ~NodeCalculator();

    void start();

    void stop();

    NodePtr result() const;

    int getCurrentSearchDepth() const;

    int getMaxSearchDepth() const;

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
    boost::scoped_ptr<NodeCalculatorImpl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATOR_H
