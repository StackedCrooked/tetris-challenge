#ifndef TETRIS_NODECALCULATORIMPL_H
#define TETRIS_NODECALCULATORIMPL_H


#include "Tetris/BlockTypes.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Evaluator.h"
#include "Tetris/NodeCalculator.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include "Futile/Assert.h"
#include "Futile/Atomic.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include "stm.hpp"
#include <boost/static_assert.hpp>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>


namespace Tetris {


class SetFinishedException : public std::runtime_error
{
public:
    SetFinishedException() :
        std::runtime_error("SetFinished failed. This is likely caused by a game-over situation.")
    {
    }

    virtual ~SetFinishedException() throw() {}
};


/// Etage stores the best scoring game state for a given etage.
/// A tree has an etage for each depth level:
///      *    Etage 0 (root)
///     ***   Etage 1
///    *****  Etage 2
///           ...
/// The higher the etage the more nodes.
class Etage
{
public:
    Etage() :
        mBestNode(NodePtr()),
        mBestQuality(0),
        mNodeCount(0),
        mFinished(bool(false))
    {
    }

    const NodePtr & bestNode(stm::transaction & tx) const
    {
        return mBestNode.open_r(tx);
    }

    std::size_t nodeCount(stm::transaction & tx) const
    { return mNodeCount.open_r(tx); }

    bool finished(stm::transaction & tx) const
    { return mFinished.open_r(tx); }

    void registerNode(stm::transaction & tx, const NodePtr & inNode, int quality)
    {
        const NodePtr & cBestNode = mBestNode.open_r(tx);
        const int & cBestQuality = mBestQuality.open_r(tx);
        if (!cBestNode || quality > cBestQuality)
        {
            mBestNode.open_rw(tx) = inNode;
            mBestQuality.open_rw(tx) = quality;
        }
        mNodeCount.open_rw(tx) += 1;
    }

    void setFinished(stm::transaction & tx)
    {
        Assert(!mFinished.open_r(tx));
        if (!mBestNode.open_r(tx))
        {
            throw SetFinishedException();
        }
        Assert(mBestNode.open_r(tx));
        mFinished.open_rw(tx) = true;
    }

private:
    mutable stm::shared<NodePtr> mBestNode;
    mutable stm::shared<int> mBestQuality;
    mutable stm::shared<std::size_t> mNodeCount;
    mutable stm::shared<bool> mFinished;
};



typedef std::vector<Etage> Etages;


class VerticalResults
{
public:
    VerticalResults(std::size_t inMaxDepth) :
        cEtages(inMaxDepth),
        mCurrentIndex(0)
    {
    }

    std::size_t depth(stm::transaction & tx) const
    {
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        return cIndex;
    }

    std::size_t maxDepth() const
    {
        return cEtages.size();
    }

    void registerNode(stm::transaction & tx, const NodePtr & inNode)
    {
        int quality = inNode->quality();
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        cEtages[cIndex].registerNode(tx, inNode, quality);
        Assert(cIndex == 0 || bestNode(tx));
    }

    const NodePtr & bestNode(stm::transaction & tx) const
    {
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        return cEtages[cIndex - 1].bestNode(tx);
    }

    bool finished(stm::transaction & tx) const
    {
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        return cIndex == maxDepth();
    }

    void setFinished(stm::transaction & tx)
    {
        std::size_t & currentIndex = mCurrentIndex.open_rw(tx);
        cEtages[currentIndex].setFinished(tx);
        currentIndex++;
        Assert(currentIndex <= maxDepth());
    }

private:
    Etages cEtages; // vector is const but not the contained elements
    mutable stm::shared<std::size_t> mCurrentIndex;
};


/**
 * NodeCalculatorImpl is the base class for the singlethreaded and multithreaded node calculators.
 */
class NodeCalculatorImpl
{
public:
    NodeCalculatorImpl(const GameState & inGameState,
                       const BlockTypes & inBlockTypes,
                       const std::vector<int> & inWidths,
                       const Evaluator & inEvaluator,
                       Futile::Worker & inMainWorker,
                       Futile::WorkerPool & inWorkerPool);

    virtual ~NodeCalculatorImpl() = 0;

    void start();

    void stop();

    int getCurrentSearchDepth() const;

    int getMaxSearchDepth() const;

    std::vector<GameState> result() const;

    int status() const;

    std::string errorMessage() const;

protected:
    virtual void populate() = 0;

    void startImpl();

    void setQuitFlag();
    bool getQuitFlag() const;

    void setStatus(int inStatus);

    void calculateResult(stm::transaction & tx);

    // Wrapper around vector with some error checking.
    // This helps us to detect errors at an early stage.
    class Results
    {
    public:
        void push_back(const GameState & inGameState)
        {
            Assert(mGameStates.empty() || mGameStates.back().id() + 1 == inGameState.id());
            mGameStates.push_back(inGameState);
        }

        void push_front(const GameState & inGameState)
        {
            Assert(mGameStates.empty() || inGameState.id() + 1 == mGameStates.front().id());
            mGameStates.push_back(inGameState);
            std::sort(mGameStates.begin(),
                      mGameStates.end(),
                      [](const GameState & lhs, const GameState & rhs){ return lhs.id() < rhs.id(); });
        }

        std::size_t size() const { return mGameStates.size(); }

        bool empty() const { return mGameStates.empty(); }

        const std::vector<GameState> & get() const { return mGameStates; }

        std::vector<GameState>::iterator begin() { return mGameStates.begin(); }
        std::vector<GameState>::iterator end() { return mGameStates.end(); }

        std::vector<GameState>::const_iterator begin() const { return mGameStates.begin(); }
        std::vector<GameState>::const_iterator end() const { return mGameStates.end(); }

    private:
        std::vector<GameState> mGameStates;
    };


    NodePtr mRootNode;
    mutable stm::shared<Results> mResults;
    VerticalResults mVerticalResults; // uses stm internally
    const BlockTypes cBlockTypes;
    const std::vector<int> cWidths;
    const Evaluator & cEvaluator;
    std::string mErrorMessage;
    Futile::Worker & mMainWorker;
    Futile::WorkerPool & mWorkerPool;
    std::atomic_bool mQuitFlag;
    std::atomic_int mStatus;
};


BOOST_STATIC_ASSERT(ATOMIC_INT_LOCK_FREE == 2);


} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H
