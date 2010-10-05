#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Tetris/AISupport.h"
#include "Tetris/BlockType.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Threading.h"
#include "Tetris/WorkerThread.h"
#include "Poco/Stopwatch.h"
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>


namespace Tetris
{

    typedef std::vector<int> Widths;

    class Player : boost::noncopyable
    {
    public:
        Player(boost::shared_ptr<WorkerThread> inWorkerThread,
               std::auto_ptr<GameStateNode> inNode,
               const BlockTypes & inBlockTypes,
               const Widths & inWidths,
               std::auto_ptr<Evaluator> inEvaluator);

        ~Player();

        void start();

        void interrupt();

        bool isFinished() const;

        int getCurrentSearchDepth() const;

        int getMaxSearchDepth() const;

        bool result(NodePtr & outChild);

        enum Status 
        {
            Status_Nil,
            Status_Started,
            Status_Finished,
            Status_Interrupted
        };

        Status status() const;

    private:
        void startImpl();

        void setStatus(Status inStatus);
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
            LayerData() : mNumItems(0), mFinished(false) {}
            NodePtr mBestChild;
            int mNumItems;
            bool mFinished;
        };

        // Stores layer info per layer index.
        std::vector<Protected<LayerData> > mLayers;
        Protected<int> mCompletedSearchDepth;

        NodePtr mNode;
        BlockTypes mBlockTypes;
        Widths mWidths;
        boost::scoped_ptr<Evaluator> mEvaluator;
        Status mStatus;
        mutable boost::mutex mStatusMutex;
        boost::scoped_ptr<Poco::Stopwatch> mStopwatch;
        mutable boost::mutex mStopwatchMutex;

        // Use static variable so that we can reuse the same thread.
        boost::shared_ptr<WorkerThread> mWorkerThread;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
