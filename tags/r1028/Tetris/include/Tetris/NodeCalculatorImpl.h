#ifndef TETRIS_NODECALCULATORIMPL_H_INCLUDED
#define TETRIS_NODECALCULATORIMPL_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Evaluator.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include <vector>
#include <memory>


namespace Tetris {


/**
 * NodeCalculatorImpl is the base class for the singlethreaded and multithreaded node calculators.
 */
class NodeCalculatorImpl
{
public:
    NodeCalculatorImpl(std::auto_ptr<GameStateNode> inNode,
                       const BlockTypes & inBlockTypes,
                       const std::vector<int> & inWidths,
                       const Evaluator & inEvaluator,
                       Futile::WorkerPool & inWorkerPool);

    virtual ~NodeCalculatorImpl() = 0;

    void start();

    void stop();

    int getCurrentSearchDepth() const;

    int getMaxSearchDepth() const;

    NodePtr result() const;

    int status() const;

    std::string errorMessage() const;

protected:
    virtual void populate() = 0;

    void startImpl();

    void setQuitFlag();
    bool getQuitFlag() const;

    void setStatus(int inStatus);

    void populateNodesRecursively(NodePtr ioNode,
                                  const BlockTypes & inBlockTypes,
                                  const std::vector<int> & inWidths,
                                  std::size_t inIndex,
                                  std::size_t inMaxIndex);

    void destroyInferiorChildren();

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

        inline NodePtr bestNode() const
        { return mBestNode; }

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
        }

        inline void setFinished()
        {
            Assert(!mFinished);
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

        void registerNode(NodePtr inNode)
        {
            Futile::ScopedLock lock(mMutex);
            mInfos.back().registerNode(inNode);
        }

        inline NodePtr bestNode() const
        {
            Futile::ScopedLock lock(mMutex);
            if (mInfos.back().finished())
            {
                return mInfos.back().bestNode();
            }
            else
            {
                return mInfos[mInfos.size() - 2].bestNode();
            }
        }

        inline bool finished() const
        {
            Futile::ScopedLock lock(mMutex);
            return mInfos.size() == mMaxDepth && mInfos.back().finished();
        }

        inline void setFinished()
        {
            Futile::ScopedLock lock(mMutex);
            mInfos.back().setFinished();
            mInfos.push_back(TreeRowInfo(*mEvaluator));
        }

    private:
        std::vector<TreeRowInfo> mInfos;
        int mMaxDepth;
        const Evaluator * mEvaluator;
        mutable Futile::Mutex mMutex;
    };

    NodePtr mNode;
    NodePtr mResult;
    mutable Futile::Mutex mNodeMutex;


    bool mQuitFlag;
    mutable Futile::Mutex mQuitFlagMutex;

    TreeRowInfos mTreeRowInfos;

    BlockTypes mBlockTypes;
    std::vector<int> mWidths;
    const Evaluator & mEvaluator;

    int mStatus;
    mutable Futile::Mutex mStatusMutex;
    std::string mErrorMessage;

    Futile::Worker mMainWorker;
    Futile::WorkerPool & mWorkerPool;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H_INCLUDED
