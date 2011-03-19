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


class NodeCalculatorImpl
{
public:
    NodeCalculatorImpl(std::auto_ptr<GameStateNode> inNode,
                       const BlockTypes & inBlockTypes,
                       const std::vector<int> & inWidths,
                       std::auto_ptr<Evaluator> inEvaluator,
                       Futile::WorkerPool & inWorkerPool);

    virtual ~NodeCalculatorImpl() = 0;

    void start();

    void stop();

    int getCurrentSearchDepth() const;

    int getMaxSearchDepth() const;

    NodePtr result() const;

    int status() const;

    const std::string & errorMessage() const;

protected:
    virtual void populate() = 0;

    void startImpl();

    void setQuitFlag();
    bool getQuitFlag() const;

    void setStatus(int inStatus);

    void populateNodesRecursively(NodePtr ioNode,
                                  const BlockTypes & inBlockTypes,
                                  const std::vector<int> & inWidths,
                                  size_t inIndex,
                                  size_t inMaxIndex);

    void destroyInferiorChildren();

    void calculateResult();

    // Store info per horizontal level of nodes.
    class TreeRowInfo
    {
    public:
        TreeRowInfo(std::auto_ptr<Evaluator> inEvaluator) :
            mBestNode(),
            mBestScore(0),
            mEvaluator(inEvaluator.release()),
            mNodeCount(0),
            mFinished(false)
        {
        }

        inline NodePtr bestNode() const
        { return mBestNode; }

        inline size_t nodeCount() const
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
        boost::shared_ptr<Evaluator> mEvaluator;
        size_t mNodeCount;
        bool mFinished;
    };

    class TreeRowInfos
    {
    public:
        TreeRowInfos(std::auto_ptr<Evaluator> inEvaluator, size_t inMaxDepth) :
            mInfos(),
            mCurrentSearchDepth(0),
            mMutex()
        {
            for (size_t idx = 0; idx != inMaxDepth; ++idx)
            {
                mInfos.push_back(TreeRowInfo(inEvaluator->clone()));
            }
        }

        inline int currentSearchDepth() const
        {
            return mCurrentSearchDepth;
        }

        inline int maximumSearchDepth() const
        {
            return mInfos.size();
        }

        inline NodePtr bestNode(size_t inDepth) const
        {
            Futile::ScopedLock lock(mMutex);
            Assert(inDepth > 0 && inDepth <= mInfos.size());
            return mInfos[inDepth - 1].bestNode();
        }

        inline NodePtr bestNode() const
        {
            NodePtr result;
            if (mCurrentSearchDepth > 0)
            {
                result = mInfos[mCurrentSearchDepth - 1].bestNode();
                Assert(result);
            }
            return result;
        }

        inline size_t nodeCount(size_t inDepth) const
        {
            Futile::ScopedLock lock(mMutex);
            return mInfos[inDepth - 1].nodeCount();
        }

        inline bool finished(size_t inDepth) const
        {
            Futile::ScopedLock lock(mMutex);
            return mInfos[inDepth - 1].finished();
        }

        void registerNode(NodePtr inNode, size_t inDepth)
        {
            Futile::ScopedLock lock(mMutex);
            mInfos[inDepth - 1].registerNode(inNode);
        }

        inline void setFinished(size_t inDepth)
        {
            if (mInfos[inDepth - 1].bestNode())
            {
                mCurrentSearchDepth = inDepth;
                mInfos[inDepth - 1].setFinished();
            }
            else
            {
                LogError(Futile::MakeString() << "TreeRowInfos::setFinished at depth " << inDepth << " failed because it does not contain any nodes.");
            }
        }

    private:
        std::vector<TreeRowInfo> mInfos;
        int mCurrentSearchDepth;
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
    boost::scoped_ptr<Evaluator> mEvaluator;

    int mStatus;
    mutable Futile::Mutex mStatusMutex;
    std::string mErrorMessage;

    Futile::Worker mMainWorker;
    Futile::WorkerPool & mWorkerPool;
};


} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H_INCLUDED
