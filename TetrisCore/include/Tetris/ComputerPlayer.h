#ifndef TETRIS_PLAYER_H_INCLUDED
#define TETRIS_PLAYER_H_INCLUDED


#include "Tetris/Game.h"
#include "Tetris/NodePtr.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Threading.h"
#include <boost/thread.hpp>


namespace Tetris
{

    class BlockMover;
    class Evaluator;
    class Gravity;
    class MoveCalculator;
    class Worker;
    class WorkerPool;
    typedef std::vector<int> Widths;


    class ComputerPlayer
    {
    public:
        ComputerPlayer(const Protected<Game> & inProtectedGame);

        void runImpl();


    private:
        int calculateRemainingTimeMs(Game & game) const;

        boost::shared_ptr<WorkerPool> mWorkerPool;
        Protected<Game> mProtectedGame;
        boost::scoped_ptr<MoveCalculator> mMoveCalculator;
        boost::scoped_ptr<Gravity> mGravity;
        boost::scoped_ptr<BlockMover> mBlockMover;
        boost::scoped_ptr<Evaluator> mEvaluator;
        int mCurrentSearchDepth;
        int mMaxSearchDepth;
        int mSearchWidth;
    };


    class MoveCalculator : boost::noncopyable
    {
    public:
        virtual ~MoveCalculator() = 0 {}

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual int getCurrentSearchDepth() const = 0;

        virtual int getMaxSearchDepth() const = 0;

        enum Status
        {
            Status_Begin,
            Status_Nil = Status_Begin,
            Status_Started,
            Status_Working,
            Status_Stopped,
            Status_Finished,
            Status_End
        };
        
        virtual Status status() const = 0;

        virtual NodePtr result() const = 0;
    };


    class CompositeMoveCalculator : public MoveCalculator
    {
    public:
        virtual ~CompositeMoveCalculator() = 0 {}

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual int getCurrentSearchDepth() const = 0;

        virtual int getMaxSearchDepth() const = 0;

        virtual NodePtr result() const = 0;

        MoveCalculator::Status status() const = 0;
    };


    class ConcreteMoveCalculator : public MoveCalculator
    {
    public:
        ConcreteMoveCalculator(boost::shared_ptr<Worker> inWorker,
                               std::auto_ptr<GameStateNode> inNode,
                               const BlockTypes & inBlockTypes,
                               const Widths & inWidths,
                               std::auto_ptr<Evaluator> inEvaluator);

        virtual ~ConcreteMoveCalculator();

        virtual void start();

        virtual void stop();

        virtual int getCurrentSearchDepth() const;

        virtual int getMaxSearchDepth() const;

        virtual NodePtr result() const;

        MoveCalculator::Status status() const;

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

        MoveCalculator::Status mStatus;
        mutable boost::mutex mStatusMutex;

        boost::shared_ptr<Worker> mWorker;
        bool mDestroyedInferiorChildren; // just for testing
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
