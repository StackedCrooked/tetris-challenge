#ifndef TETRIS_NODECALCULATORIMPL_H_INCLUDED
#define TETRIS_NODECALCULATORIMPL_H_INCLUDED


#include "Tetris/BlockTypes.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/WorkerPool.h"
#include <boost/shared_ptr.hpp>
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
                           boost::shared_ptr<WorkerPool> inWorkerPool);

        virtual ~NodeCalculatorImpl() = 0;

        void start();

        void stop();

        int getCurrentSearchDepth() const;

        int getMaxSearchDepth() const;

        NodePtr result() const;

        int status() const;

        // LayerData contains the accumulated data for all branches at a same depth.
        struct LayerData
        {
            LayerData() :
                mBestChild(),
                mNumItems(0),
                mFinished(false)
            {
            }

            NodePtr mBestChild;
            int mNumItems;
            bool mFinished;
        };

        void getLayerData(int inIndex, LayerData & outLayerData);

    protected:
        virtual void populate() = 0;

        void startImpl();

        void setStatus(int inStatus);

        void setCurrentSearchDepth(int inDepth);

        void updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount);

        void populateNodesRecursively(NodePtr ioNode,
                                      const BlockTypes & inBlockTypes,
                                      const std::vector<int> & inWidths,
                                      size_t inIndex,
                                      size_t inMaxIndex);

        void markTreeRowAsFinished(size_t inIndex);

        void destroyInferiorChildren();

        NodePtr mNode;
        mutable boost::mutex mNodeMutex;

        // Store info per horizontal level of nodes.
        std::vector<LayerData> mLayers;
        mutable boost::mutex mLayersMutex;

        int mCompletedSearchDepth;
        mutable boost::mutex mCompletedSearchDepthMutex;

        BlockTypes mBlockTypes;
        std::vector<int> mWidths;
        boost::scoped_ptr<Evaluator> mEvaluator;

        int mStatus;
        mutable boost::mutex mStatusMutex;

        boost::shared_ptr<WorkerPool> mWorkerPool;
    };

} // namespace Tetris


#endif // TETRIS_NODECALCULATORIMPL_H_INCLUDED
