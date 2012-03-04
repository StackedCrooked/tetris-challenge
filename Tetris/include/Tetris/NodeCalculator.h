#ifndef TETRIS_NODECALCULATOR_H
#define TETRIS_NODECALCULATOR_H


#include "Tetris/AISupport.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/NodePtr.h"
#include "Futile/MakeString.h"
#include "Futile/WorkerPool.h"
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>


namespace Tetris {


class Evaluator;
class GameStateNode;
class NodeCalculatorImpl;
typedef std::vector<int> Widths;
using namespace Futile;


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

    //! Returns current results.
    std::vector<GameState> results() const;

    //! Returns current depth / max depth.
    Progress progress() const;

    Progress nodeCount() const;

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

    static std::string ConvertStatusToString(Status inStatus)
    {
        switch (inStatus)
        {
            case Status_Initial: return "Initial";
            case Status_Starting: return "Starting";
            case Status_Working: return "Working";
            case Status_Stopping: return "Stopping";
            case Status_Finished: return "Finished";
            case Status_Error: return "Error";
            default: throw std::logic_error(SS() << "Invalid Status: " << int(inStatus));
        }
    }

    // Returns the error message (in case of Status_Error).
    std::string errorMessage() const;

private:
    boost::scoped_ptr<NodeCalculatorImpl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATOR_H
