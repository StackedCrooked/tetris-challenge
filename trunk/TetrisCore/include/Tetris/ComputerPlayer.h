#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Tetris/AISupport.h"
#include "Tetris/BlockType.h"
#include "Tetris/Threading.h"
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>


namespace Tetris
{


    class GameStateNode;
    class WorkerThread;
    typedef std::vector<int> Widths;


    class ComputerPlayer : boost::noncopyable
    {
    public:
        ComputerPlayer(boost::shared_ptr<WorkerThread> inWorkerThread,
                       std::auto_ptr<GameStateNode> inNode,
                       const BlockTypes & inBlockTypes,
                       const Widths & inWidths,
                       std::auto_ptr<Evaluator> inEvaluator);

        ~ComputerPlayer();

        void start();

        void stop();

        bool isFinished() const;

        int getCurrentSearchDepth() const;

        int getMaxSearchDepth() const;

        NodePtr result() const;

        enum Status
        {
            Status_Nil,
            Status_Started,
            Status_Finished
        };

        Status status() const;

    private:
        void startImpl();

        void setStatus(Status inStatus);

        void setCurrentSearchDepth(int inDepth);

        void updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount);

        void markTreeRowAsFinished(size_t inIndex);
        void populate();
        void destroyInferiorChildren();

        void populateNodesRecursively(NodePtr ioNode,
                                      const BlockTypes & inBlockTypes,
                                      const Widths & inWidths,
                                      size_t inIndex,
                                      size_t inMaxIndex);

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

        NodePtr mNode;
        mutable boost::mutex mNodeMutex;

        // Store info per horizontal level of nodes.
        std::vector<LayerData> mLayers;
        mutable boost::mutex mLayersMutex;

        int mCompletedSearchDepth;
        mutable boost::mutex mCompletedSearchDepthMutex;


        BlockTypes mBlockTypes;
        Widths mWidths;
        boost::scoped_ptr<Evaluator> mEvaluator;

        Status mStatus;
        mutable boost::mutex mStatusMutex;

        boost::shared_ptr<WorkerThread> mWorkerThread;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
