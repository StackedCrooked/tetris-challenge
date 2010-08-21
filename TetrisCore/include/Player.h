#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "Threading.h"
#include "ThreadSafeGame.h"
#include "JobList.h"
#include "Poco/Stopwatch.h"
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
        static const int cMaxDepth = 10;
        Player(std::auto_ptr<GameStateNode> inNode,
               const BlockTypes & inBlockTypes,
               int inTimeLimitMs);


        // Starts recursively populating the child nodes.
        // Uses a separate thread for each immediate child of the starting node.
        // Given a game grid width of 10 columns, this means that at least 9
        // and at most 32 threads will be running simultaneously.
        //
        // Blocking! Call this method in a separate thread to avoid blocking the main game.
        void start();

        inline void stop() { mStop = true; }

        int remainingTimeMs() const;

        GameStateNode * node() { return mNode.get(); }

        const GameStateNode * node() const { return mNode.get(); }

        // Returns best child and its depth.
        ChildNodePtr getBestChild() const;

        size_t getCurrentDepth() const;

    private:
        bool isTimeExpired();

        // We use a ChildNodePtr (shared_ptr) to prevent destruction of the
        // object during program exit while this thread is still accessing it.
        void populateNodesInBackground(ChildNodePtr ioNode,
                                       BlockTypes * inBlockTypes,
                                       size_t inDepth);

        void populateNodesRecursively(GameStateNode & ioNode, const BlockTypes & inBlockTypes, size_t inDepth);
        void addToFlattenedNodes(const ChildNodes & inChildNode, size_t inDepth);
        void commitThreadLocalData();
        void onTimer(Poco::Timer &);

        boost::scoped_ptr<GameStateNode> mNode;
        BlockTypes mBlockTypes;

        struct Result
        {
        public:
            Result() :
                mChildNodesPerDepth(cMaxDepth)
            {
            }

            void mergeAtDepth(int inDepth, const ChildNodes & inChildNodes)
            {
                ChildNodes & result = mChildNodesPerDepth[inDepth];
                for (ChildNodes::const_iterator it = inChildNodes.begin(); it != inChildNodes.end(); ++it)
                {
                    result.insert(*it);
                }
            }

            size_t sizeAtDepth(size_t inDepth) const
            {
                return mChildNodesPerDepth[inDepth].size();
            }

            const ChildNodes & getNodesAtDepth(size_t inDepth) const
            {
                return mChildNodesPerDepth[inDepth];
            }

        private:
            std::vector<ChildNodes> mChildNodesPerDepth;
        };

        Protected<Result> mResult;


        typedef boost::thread_specific_ptr<Result> ThreadLocalResult;
        ThreadLocalResult mThreadLocalResult;

        Poco::Timer mTimer;
        int mTimeLimitMs;
        Poco::Stopwatch mStopwatch;
        volatile bool mStop;
        boost::thread_group mThreadPool;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
