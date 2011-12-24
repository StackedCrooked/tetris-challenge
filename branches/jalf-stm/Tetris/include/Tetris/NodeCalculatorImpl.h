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
#include "stm.hpp"
#include <cstddef>
#include <memory>
#include <vector>
#include <stack>


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

    void destroyInferiorChildren();

    void calculateResult();

    // Store info per horizontal level of nodes.
    class TreeLevel
    {
    public:
        TreeLevel(const Evaluator & inEvaluator) :
            mBestNode(),
            mBestScore(0),
            mEvaluator(&inEvaluator),
            mNodeCount(0),
            mFinished(false)
        {
        }

        inline NodePtr bestNode() const
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

        void registerNode(NodePtr inNode)
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

    class TreeLevelStack
    {
    private:
        typedef std::stack<TreeLevel> Stack;
        typedef stm::shared<Stack> SharedStack;

        inline static Stack GetInitialStack(const Evaluator & inEvaluator)
        {
            Stack result;
            result.push(TreeLevel(inEvaluator));
            return result;
        }

    public:
        TreeLevelStack(const Evaluator & inEvaluator, std::size_t inMaxDepth) :
            mSharedStack(GetInitialStack(inEvaluator)),
            mMaxDepth(inMaxDepth),
            mEvaluator(&inEvaluator)
        {
        }

        inline unsigned depth() const
        {
            return stm::atomic<unsigned>([&](stm::transaction & tx) {
                const Stack & stack = mSharedStack.open_r(tx);
                if (stack.top().finished())
                {
                    return stack.size();
                }
                else
                {
                    return stack.size() - 1;
                }
            });
        }

        inline unsigned maxDepth() const
        {
            return mMaxDepth;
        }

        void registerNode(NodePtr inNode)
        {
            stm::atomic([&](stm::transaction & tx) {
               Stack & stack = mSharedStack.open_rw(tx);
               stack.top().registerNode(inNode);
            });
        }

        inline NodePtr bestNode() const
        {
            return stm::atomic<NodePtr>([&](stm::transaction & tx) {
                const Stack & stack = mSharedStack.open_r(tx);
                return stack.top().bestNode();
            });
        }

        inline bool finished() const
        {
            return stm::atomic<bool>([&](stm::transaction & tx) {
                const Stack & stack = mSharedStack.open_r(tx);
                return stack.size() == (mMaxDepth - 1) && stack.top().finished();
            });
        }

        inline void setFinished()
        {
            stm::atomic([&](stm::transaction & tx) {
                Stack & stack = mSharedStack.open_rw(tx);
                Assert(!stack.empty());
                stack.top().setFinished();
                if (stack.size() < mMaxDepth)
                {
                    stack.push(TreeLevel(*mEvaluator));
                }
            });
        }

    private:
        mutable SharedStack mSharedStack;
        const std::size_t mMaxDepth;
        const Evaluator * mEvaluator;
    };

    TreeLevelStack mTreeRowInfos;


    NodePtr mNode;
    std::vector<GameState> mResult;
    mutable Futile::Mutex mNodeMutex;

    bool mQuitFlag;
    mutable Futile::Mutex mQuitFlagMutex;


    BlockTypes mBlockTypes;
    std::vector<int> mWidths;
    const Evaluator & mEvaluator;

    int mStatus;
    mutable Futile::Mutex mStatusMutex;
    std::string mErrorMessage;

    Futile::Worker & mMainWorker;
    Futile::WorkerPool & mWorkerPool;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H
