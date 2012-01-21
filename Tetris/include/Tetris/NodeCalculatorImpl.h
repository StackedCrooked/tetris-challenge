#ifndef TETRIS_NODECALCULATORIMPL_H
#define TETRIS_NODECALCULATORIMPL_H


#include "Tetris/BlockTypes.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Evaluator.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include "Futile/Atomic.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include <boost/static_assert.hpp>
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>


namespace Tetris {


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

    void calculateResult();

    // Store info per horizontal level of nodes.
    class TreeRowInfo
    {
    public:
        TreeRowInfo(const Evaluator & inEvaluator) :
            mBestNode(),
            mBestScore(0),
            mEvaluator(&inEvaluator),
            mNodeCount(0),
            mFinished(false)
        {
        }

        inline const NodePtr & bestNode() const
        {
            if (!mBestNode)
            {
                throw std::runtime_error("Best node is null.");
            }
            return mBestNode;
        }

        inline std::size_t nodeCount() const
        { return mNodeCount; }

        inline bool finished() const
        { return mFinished; }

        void registerNode(const NodePtr & inNode)
        {
            int score = mEvaluator->evaluate(inNode->gameState());
            if (!mBestNode || score > mBestScore)
            {
                mBestNode = inNode;
                mBestScore = score;
            }
            mNodeCount++;
            Assert(mBestNode);
        }

        inline void setFinished()
        {
            Assert(!mFinished);
            Assert(mBestNode);
            mFinished = true;
        }

    private:
        NodePtr mBestNode;
        int mBestScore;
        const Evaluator * mEvaluator;
        std::size_t mNodeCount;
        bool mFinished;
    };

    class TreeRowInfos
    {
    public:
        TreeRowInfos(const Evaluator & inEvaluator, std::size_t inMaxDepth) :
            mInfos(),
            mMaxDepth(inMaxDepth),
            mEvaluator(&inEvaluator),
            mMutex()
        {
            mInfos.push_back(TreeRowInfo(*mEvaluator));
        }

        inline std::size_t depth() const
        {
            Futile::ScopedLock lock(mMutex);
            if (mInfos.back().finished())
            {
                return mInfos.size();
            }
            else
            {
                Assert(mInfos.size() >= 1);
                return mInfos.size() - 1;
            }
        }

        inline std::size_t maxDepth() const
        {
            Futile::ScopedLock lock(mMutex);
            return mMaxDepth;
        }

        void registerNode(const NodePtr & inNode)
        {
            Futile::ScopedLock lock(mMutex);
            mInfos.back().registerNode(inNode);
            Assert(bestNode());
        }

        inline const NodePtr & bestNode() const
        {
            Futile::ScopedLock lock(mMutex);
            if (mInfos.empty())
            {
                throw std::runtime_error("There is no best node.");
            }

            if (mInfos.size() >= 2)
            {
                return mInfos[mInfos.size() - 2].bestNode();
            }
            else
            {
                return mInfos[mInfos.size() - 1].bestNode();
            }

            throw std::runtime_error("Invalid state.");
        }

        inline bool finished() const
        {
            Futile::ScopedLock lock(mMutex);
            return mInfos.size() == (mMaxDepth - 1) && mInfos.back().finished();
        }

        inline void setFinished()
        {
            Futile::ScopedLock lock(mMutex);
            Assert(!mInfos.empty());
            mInfos.back().setFinished();
            mInfos.push_back(TreeRowInfo(*mEvaluator));
        }

    private:
        std::vector<TreeRowInfo> mInfos;
        std::size_t mMaxDepth;
        const Evaluator * mEvaluator;
        mutable Futile::Mutex mMutex;
    };

    NodePtr mNode;
    std::vector<GameState> mResult;
    std::atomic_bool mQuitFlag;
    std::atomic_int mStatus;
    TreeRowInfos mTreeRowInfos;
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
