#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "Threading.h"
#include "ThreadSafeGame.h"
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
        Player(std::auto_ptr<GameStateNode> inNode,
               const BlockTypes & inBlockTypes);


        void start();

        bool isFinished() const;

        ChildNodePtr result();

    private:
        void startImpl();

        void populateNodesRecursively(GameStateNode & ioNode,
                                      const BlockTypes & inBlockTypes,
                                      size_t inDepth);

        ChildNodePtr mNode;
        ChildNodePtr mEndNode;
        BlockTypes mBlockTypes;
        volatile bool mIsFinished;
        boost::scoped_ptr<boost::thread> mThread;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
