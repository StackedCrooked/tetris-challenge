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

    void start(stm::transaction & tx);

    void stop(stm::transaction & tx);

    int getCurrentSearchDepth(stm::transaction & tx) const;

    int getMaxSearchDepth() const;

    std::vector<GameState> result(stm::transaction & tx) const;

    int status(stm::transaction & tx) const;

    std::string errorMessage(stm::transaction & tx) const;

protected:
    virtual void populate() = 0;

    void startImpl();

    void setQuitFlag(stm::transaction & tx);
    bool getQuitFlag(stm::transaction & tx) const;

    void setStatus(stm::transaction & tx, int inStatus);

    void destroyInferiorChildren(stm::transaction & tx);

    void calculateResult(stm::transaction & tx);

    // Store info per horizontal level of nodes.
    class TreeRowInfo
    {
    public:
        TreeRowInfo(const Evaluator & inEvaluator) :
            mBestNode(),
            mBestScore(0),
            mNodeCount(0),
            mFinished(false),
            mEvaluator(const_cast<Evaluator*>(&inEvaluator))
        {
        }

        TreeRowInfo(const TreeRowInfo & rhs) :
            mBestNode(rhs.mBestNode),
            mBestScore(rhs.mBestScore),
            mNodeCount(rhs.mNodeCount),
            mFinished(rhs.mFinished),
            mEvaluator(rhs.mEvaluator)
        {
        }

        TreeRowInfo & operator=(TreeRowInfo rhs) // by value (!)
        {
            swap(rhs);
            return *this;
        }

        void swap(TreeRowInfo & rhs)
        {
            std::swap(mBestNode, rhs.mBestNode);
            std::swap(mBestScore, rhs.mBestScore);
            std::swap(mNodeCount, rhs.mNodeCount);
            std::swap(mFinished, rhs.mFinished);
            std::swap(mEvaluator, rhs.mEvaluator);
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
        std::size_t mNodeCount;
        bool mFinished;
        Evaluator * mEvaluator;
    };

    class TreeRowInfos
    {
    private:
        typedef std::vector<TreeRowInfo> Infos;
        mutable stm::shared<Infos> mInfos;
        std::size_t mMaxDepth;
        Evaluator * mEvaluator;

    public:
        TreeRowInfos(const Evaluator & inEvaluator, std::size_t inMaxDepth) :
            mInfos(Infos(1, TreeRowInfo(inEvaluator))),
            mMaxDepth(inMaxDepth),
            mEvaluator(const_cast<Evaluator*>(&inEvaluator))
        {
        }

        TreeRowInfos(const TreeRowInfos & rhs) :
            mInfos(rhs.mInfos),
            mMaxDepth(rhs.mMaxDepth),
            mEvaluator(rhs.mEvaluator)
        {
        }

        TreeRowInfos& operator=(TreeRowInfos rhs) // by value (!)
        {
            swap(rhs);
            return *this;
        }

        void swap(TreeRowInfos & rhs)
        {
            stm::atomic([&](stm::transaction & tx) {
                Infos & leftInfos = mInfos.open_rw(tx);
                Infos & rightInfos = rhs.mInfos.open_rw(tx);
                leftInfos.swap(rightInfos);
                std::swap(mMaxDepth, rhs.mMaxDepth);
                std::swap(mEvaluator, rhs.mEvaluator);
            });
        }

        inline std::size_t depth(stm::transaction & tx) const
        {
            const Infos & infos = mInfos.open_r(tx);
            if (infos.back().finished())
            {
                return infos.size();
            }
            else
            {
                Assert(infos.size() >= 1);
                return infos.size() - 1;
            }
        }

        inline std::size_t maxDepth() const
        {
            return mMaxDepth;
        }

        void registerNode(stm::transaction & tx, NodePtr inNode)
        {
            Infos & infos = mInfos.open_rw(tx);
            infos.back().registerNode(inNode);
        }

        inline NodePtr bestNode(stm::transaction & tx) const
        {
            const Infos & infos = mInfos.open_r(tx);
            if (infos.empty())
            {
                throw std::runtime_error("There is no best node.");
            }

            if (infos.size() >= 2)
            {
                return infos[infos.size() - 2].bestNode();
            }
            else
            {
                return infos[infos.size() - 1].bestNode();
            }

            throw std::runtime_error("Invalid state.");
        }

        inline bool finished(stm::transaction & tx) const
        {
            const Infos & infos = mInfos.open_r(tx);
            return infos.size() == (mMaxDepth - 1) && infos.back().finished();
        }

        inline void setFinished(stm::transaction & tx)
        {
            Infos & infos = mInfos.open_rw(tx);
            Assert(!infos.empty());
            infos.back().setFinished();
            infos.push_back(TreeRowInfo(*mEvaluator));
        }
    };

    static void testcopy()
    {
        TreeRowInfos test(ConcreteEvaluator<MakeTetrises>::Instance(), 1);
        TreeRowInfos test2 = test;
        test = test2;
    }

    typedef std::vector<GameState> Result;

    NodePtr mNode; // TODO: make thread-safe!
    mutable stm::shared<Result> mResult;
    mutable stm::shared<bool> mQuitFlag;
    mutable stm::shared<TreeRowInfos> mTreeRowInfos;

    const BlockTypes mBlockTypes;
    const std::vector<int> mWidths;
    const Evaluator & mEvaluator;

    mutable stm::shared<int> mStatus;
    mutable stm::shared<std::string> mErrorMessage;

    Futile::Worker & mMainWorker;
    Futile::WorkerPool & mWorkerPool;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H
