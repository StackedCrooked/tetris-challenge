#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "ThreadSafeGame.h"
#include "JobList.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <ostream>
#include <vector>


namespace Tetris
{
    
    typedef std::vector<int> Widths;

    class Player
    {
    public:
        Player(const ThreadSafeGame & inThreadSafeGame);

        void move(const std::vector<int> & inWidths);

        void setLogger(std::ostream & inOutStream);

        void playUntilGameOver(const std::vector<int> & inWidths);

    private:
        void log(const std::string & inMessage);

        ThreadSafeGame mThreadSafeGame;
        std::ostream * mOutStream;
    };


    class TimedNodePopulator
    {
    public:
        TimedNodePopulator(std::auto_ptr<GameStateNode> inNode,
                           const BlockTypes & inBlockTypes,
                           int inTimeMs);

        void start();

        bool isTimeExpired() const;

    private:
        void populateNode(std::auto_ptr<GameStateNode> ioNode);
        void populateNodesInBackground(GameStateNode * ioNode, BlockTypes * inBlockTypes, size_t inOffset);
        void populateNodesRecursively(GameStateNode & ioNode, const BlockTypes & inBlockTypes, size_t inOffset);
        void addToFlattenedNodes(std::auto_ptr<GameStateNode> inNode, size_t inOffset);

        boost::scoped_ptr<GameStateNode> mNode;
        BlockTypes mBlockTypes;
        Poco::Int64 mTimeMicroseconds;
        
        std::vector<ChildNodes> mFlattenedNodes;
        mutable boost::mutex mFlattenedNodesMutex;        

        Poco::Stopwatch mStopwatch;
        mutable boost::mutex mStopwatchMutex;

        boost::thread_group mThreadPool;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
