#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Threading.h"
#include "Poco/Stopwatch.h"
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>


namespace Tetris
{

    
    typedef std::vector<int> Widths;
    void DestroyInferiorChildren(GameStateNode * srcNode, GameStateNode * dstNode);


    class Player : boost::noncopyable
    {
    public:
        Player(std::auto_ptr<GameStateNode> inNode,
               const BlockTypes & inBlockTypes,
               const std::vector<size_t> & inWidths,
               std::auto_ptr<Evaluator> inEvaluator);

        ~Player();

        void start();

        bool isFinished() const;

        int getCurrentSearchDepth() const;

        bool result(NodePtr & outChild);

        enum Status 
        {
            Status_Null,
            Status_Calculating,
            Status_Finished,
            Status_Interrupted
        };

        Status getStatus() const;

        void setStatus(Status inStatus);

    private:
        void startImpl();

        
        void markTreeRowAsFinished(size_t inIndex);
        void populate();
        void destroyInferiorChildren();

        void populateNodesRecursively(NodePtr ioNode,
                                      const BlockTypes & inBlockTypes,
                                      const std::vector<size_t> & inWidths,
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
        std::vector<size_t> mWidths;
        boost::scoped_ptr<Evaluator> mEvaluator;
        Status mStatus;
        mutable boost::mutex mStatusMutex;
        boost::scoped_ptr<Poco::Stopwatch> mStopwatch;
        mutable boost::mutex mStopwatchMutex;
        boost::scoped_ptr<boost::thread> mThread;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
