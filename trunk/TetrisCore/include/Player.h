#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "Threading.h"
#include "ThreadSafeGame.h"
#include "JobList.h"
#include "Poco/Timer.h"
#include "Poco/Types.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <ostream>
#include <vector>


namespace Tetris
{
    
    typedef std::vector<int> Widths;
    void DestroyInferiorChildren(GameStateNode * srcNode, GameStateNode * dstNode);

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
        static const int cMaxDepth = 10;
        TimedNodePopulator(std::auto_ptr<GameStateNode> inNode,
                           const BlockTypes & inBlockTypes,
                           int inTimeMs);


        // Starts recursively populating the child nodes.
        // Uses a separate thread for each immediate child of the starting node.
        // Given a game grid width of 10 columns, this means that at least 9
        // and at most 32 threads will be running simultaneously.
        //
        // Blocking! Call this method in a separate thread to avoid blocking the main game.
        void start();

        GameStateNode * node() { return mNode.get(); }

        const GameStateNode * node() const { return mNode.get(); }

        bool isTimeExpired() const;

        // Returns best child and its depth.
        ChildNodePtr getBestChild() const;

        size_t getCurrentDepth() const;

    private:
        void populateNodesInBackground(GameStateNode * ioNode, BlockTypes * inBlockTypes, size_t inOffset);
        void populateNodesRecursively(GameStateNode & ioNode, const BlockTypes & inBlockTypes, size_t inOffset);
        void addToFlattenedNodes(ChildNodePtr inChildNode, size_t inOffset);
        void onTimer(Poco::Timer &);

        boost::scoped_ptr<GameStateNode> mNode;
        BlockTypes mBlockTypes;
        Poco::Int64 mTimeMicroseconds;

        Protected<ChildNodes> mResults[cMaxDepth];

        Poco::Timer mTimer;
        volatile bool mIsTimeExpired;
        boost::thread_group mThreadPool;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
