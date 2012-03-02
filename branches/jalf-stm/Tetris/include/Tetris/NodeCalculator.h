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

    typedef std::function<void(const std::vector<GameState> &)> Callback;

    void start(const Callback & inCallback);

    void stop();

    std::vector<GameState> result() const;

    int getCurrentSearchDepth() const;

    int getMaxSearchDepth() const;

    unsigned getCurrentNodeCount() const;

    unsigned getMaxNodeCount() const;

    enum Status
    {
        Status_Begin,
        Status_Initial = Status_Begin,
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
