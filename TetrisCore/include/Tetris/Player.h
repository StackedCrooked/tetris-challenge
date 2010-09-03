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
               std::auto_ptr<Evaluator> inEvaluator);

        ~Player();

        void start();

        bool isFinished() const;

		bool isGameOver() const;

        bool result(ChildNodePtr & outChild);

        enum Status 
        {
            Status_Null,
            Status_Calculating,
            Status_Finished,
            Status_Interrupted,
            Status_Destructing
        };

        Status getStatus() const;

        void setStatus(Status inStatus);

    private:
        void startImpl();

        
        void markTreeRowAsFinished(size_t inIndex);
        void populateBreadthFirst();
        void destroyInferiorChildren();

        void populateNodesRecursively(GameStateNode & ioNode,
                                      const BlockTypes & inBlockTypes,
                                      size_t inIndex,
                                      size_t inMaxIndex);

        // A 'tree row' is the union of all nodes at a certain depth.
        struct TreeRowInfo
        {
            TreeRowInfo() : mNumItems(0), mFinished(false) {}
            ChildNodePtr mBestChild;
            int mNumItems;
            bool mFinished;
        };

        // Stores tree row info for each depth.
        std::vector<Protected<TreeRowInfo> > mTreeRows;

        ChildNodePtr mNode;
        BlockTypes mBlockTypes;
        boost::scoped_ptr<Evaluator> mEvaluator;
        Status mStatus;
        mutable boost::mutex mStatusMutex;
        boost::scoped_ptr<Poco::Stopwatch> mStopwatch;
        mutable boost::mutex mStopwatchMutex;
        boost::scoped_ptr<boost::thread> mThread;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
