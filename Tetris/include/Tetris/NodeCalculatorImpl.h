#ifndef TETRIS_NODECALCULATORIMPL_H
#define TETRIS_NODECALCULATORIMPL_H


#include "Tetris/BlockTypes.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Evaluator.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include "Futile/Assert.h"
#include "Futile/Atomic.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include "stm.hpp"
#include <boost/static_assert.hpp>
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>


namespace Tetris {


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
        mBestScore(0),
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

    void registerNode(stm::transaction & tx, const NodePtr & inNode, int score)
    {
        const NodePtr & cBestNode = mBestNode.open_r(tx);
        const int & cBestScore = mBestScore.open_r(tx);
        if (!cBestNode || score > cBestScore)
        {
            mBestNode.open_rw(tx) = inNode;
            mBestScore.open_rw(tx) = score;
        }
        mNodeCount.open_rw(tx) += 1;
    }

    void setFinished(stm::transaction & tx)
    {
        Assert(!mFinished.open_r(tx));
        Assert(mBestNode.open_r(tx));
        mFinished.open_rw(tx) = true;
    }

private:
    mutable stm::shared<NodePtr> mBestNode;
    mutable stm::shared<int> mBestScore;
    mutable stm::shared<std::size_t> mNodeCount;
    mutable stm::shared<bool> mFinished;
};



typedef std::vector<Etage> Etages;


class AllResults
{
public:
    AllResults(const Evaluator & inEvaluator, std::size_t inMaxDepth) :
        mEtages(inMaxDepth),
        mMaxDepth(mEtages.size()),
        mCurrentIndex(0),
        mEvaluator(&inEvaluator)
    {
    }

    std::size_t depth(stm::transaction & tx) const
    {
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        return cIndex;
    }

    std::size_t maxDepth() const
    {
        return mMaxDepth;
    }

    void registerNode(stm::transaction & tx, const NodePtr & inNode)
    {
        int score = mEvaluator->evaluate(inNode->gameState());
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        mEtages.at(cIndex).registerNode(tx, inNode, score);
        Assert(bestNode(tx));
    }

    const NodePtr & bestNode(stm::transaction & tx) const
    {
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        return mEtages.at(cIndex).bestNode(tx);
    }

    bool finished(stm::transaction & tx) const
    {
        const std::size_t & cIndex = mCurrentIndex.open_r(tx);
        return cIndex == mMaxDepth;
    }

    void setFinished(stm::transaction & tx)
    {
        std::size_t & currentIndex = mCurrentIndex.open_rw(tx);
        mEtages.at(currentIndex).setFinished(tx);
        currentIndex++;
        Assert(currentIndex <= mMaxDepth);
    }

private:
    Etages mEtages;
    const std::size_t mMaxDepth;
    mutable stm::shared<std::size_t> mCurrentIndex;
    const Evaluator * mEvaluator;
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

    NodePtr mNode;
    std::vector<GameState> mResult;
    std::atomic_bool mQuitFlag;
    std::atomic_int mStatus;
    AllResults mAllResults;
    BlockTypes mBlockTypes;
    std::vector<int> mWidths;
    const Evaluator & mEvaluator;
    std::string mErrorMessage;
    Futile::Worker & mMainWorker;
    Futile::WorkerPool & mWorkerPool;
};


BOOST_STATIC_ASSERT(ATOMIC_INT_LOCK_FREE == 2);


} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H
