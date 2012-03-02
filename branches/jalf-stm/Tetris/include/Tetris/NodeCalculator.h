#ifndef TETRIS_NODECALCULATOR_H
#define TETRIS_NODECALCULATOR_H


#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"
#include "Futile/WorkerPool.h"
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>


namespace Tetris {


class Evaluator;
class GameStateNode;
class NodeCalculatorImpl;
typedef std::vector<int> Widths;


class NodeCalculator : boost::noncopyable
{
public:
    NodeCalculator(const GameState & inGameState,
                   const BlockTypes & inBlockTypes,
                   const Widths & inWidths,
                   const Evaluator & inEvaluator,
                   Futile::Worker & inMainWorker,
                   Futile::WorkerPool & inWorkerPool);

    ~NodeCalculator();

    void start();

    void stop();

    // Returns curent results.
    std::vector<GameState> getCurrentResults() const;

    int getCurrentSearchDepth() const;

    int getMaxSearchDepth() const;

    unsigned getCurrentNodeCount() const;

    unsigned getMaxNodeCount() const;

    enum Status
    {
        Status_Initial,
        Status_Starting,
        Status_Working,
        Status_Stopping,
        Status_Finished,
        Status_Error = -1
    };

    Status status() const;

    // Returns the error message (in case of Status_Error).
    std::string errorMessage() const;

private:
    boost::scoped_ptr<NodeCalculatorImpl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATOR_H
