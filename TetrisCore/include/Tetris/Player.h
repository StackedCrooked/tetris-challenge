#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Tetris/GameStateNode.h"
#include "Tetris/BlockType.h"
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
               std::auto_ptr<Evaluator> inEvaluator,
               int inTimeLimitMs);

        ~Player();

        void start();

        int timeRemaining() const;

        bool isFinished() const;

		bool isGameOver() const;

        bool result(ChildNodePtr & outChild);

        enum Status 
        {
            Status_Null,
            Status_Calculating,
            Status_Finished,
            Status_TimeExpired,
            Status_Destructing
        };

        Status getStatus() const;

        void setStatus(Status inStatus);

    private:
        void startImpl();

        void populateNodesRecursively(GameStateNode & ioNode,
                                      const BlockTypes & inBlockTypes,
                                      size_t inDepth);

        ChildNodePtr mNode;
        ChildNodePtr mEndNode;
        BlockTypes mBlockTypes;
        boost::scoped_ptr<Evaluator> mEvaluator;
        int mTimeLimitMs;
        Status mStatus;
        mutable boost::mutex mStatusMutex;
        boost::scoped_ptr<Poco::Stopwatch> mStopwatch;
        mutable boost::mutex mStopwatchMutex;
        boost::scoped_ptr<boost::thread> mThread;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
