#ifndef TETRIS_NODECALCULATORIMPL_H_INCLUDED
#define TETRIS_NODECALCULATORIMPL_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/Worker.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Assert.h"
#include "Poco/Types.h"
#include <vector>
#include <memory>


namespace Tetris
{

    class NodeCalculatorImpl
    {
    public:
        NodeCalculatorImpl(std::auto_ptr<GameStateNode> inNode,
                           const BlockTypes & inBlockTypes,
                           const std::vector<int> & inWidths,
                           std::auto_ptr<Evaluator> inEvaluator,
                           WorkerPool & inWorkerPool);

        virtual ~NodeCalculatorImpl() = 0;

        void start();

        void stop();

        int getCurrentSearchDepth() const;

        int getMaxSearchDepth() const;

        NodePtr result() const;

        int status() const;

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

            inline Poco::UInt64 nodeCount() const
            { return mNodeCount; }

            inline bool finished() const
            { return mFinished; }

            void registerNode(NodePtr inNode)
            {
                int score = mEvaluator->evaluate(inNode->state());
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
                Assert(mBestNode);
                mFinished = true;
            }

        private:
            NodePtr mBestNode;
            int mBestScore;
            boost::shared_ptr<Evaluator> mEvaluator;
            Poco::UInt64 mNodeCount;
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
                boost::mutex::scoped_lock lock(mMutex);
                Assert(inDepth > 0 && inDepth <= mInfos.size());
                return mInfos[inDepth - 1].bestNode();
            }

            inline NodePtr bestNode() const
            {
                Assert(mCurrentSearchDepth > 0 && mCurrentSearchDepth <= mInfos.size());
                NodePtr result;
                if (mCurrentSearchDepth > 0)
                {
                    result = mInfos[mCurrentSearchDepth - 1].bestNode();
                    Assert(result);
                }
                return result;
            }

            inline Poco::UInt64 nodeCount(size_t inDepth) const
            {
                boost::mutex::scoped_lock lock(mMutex);
                Assert(inDepth <= mInfos.size());
                return mInfos[inDepth - 1].nodeCount();
            }

            inline bool finished(size_t inDepth) const
            {
                boost::mutex::scoped_lock lock(mMutex);
                Assert(inDepth <= mInfos.size());
                return mInfos[inDepth - 1].finished();
            }

            void registerNode(NodePtr inNode, size_t inDepth)
            {
                boost::mutex::scoped_lock lock(mMutex);
                Assert(inDepth <= mInfos.size());
                Assert(inNode);
                mInfos[inDepth - 1].registerNode(inNode);
            }

            inline void setFinished(size_t inDepth)
            {
                Assert(inDepth <= mInfos.size());
                mCurrentSearchDepth = inDepth;
                mInfos[inDepth - 1].setFinished();
            }

        private:
            std::vector<TreeRowInfo> mInfos;
            int mCurrentSearchDepth;
            mutable boost::mutex mMutex;
        };

        NodePtr mNode;
        mutable boost::mutex mNodeMutex;

        bool mQuitFlag;
        mutable boost::mutex mQuitFlagMutex;

        TreeRowInfos mTreeRowInfos;

        BlockTypes mBlockTypes;
        std::vector<int> mWidths;
        boost::scoped_ptr<Evaluator> mEvaluator;

        int mStatus;
        mutable boost::mutex mStatusMutex;

        Worker mMainWorker;
        WorkerPool & mWorkerPool;
    };

} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H_INCLUDED
