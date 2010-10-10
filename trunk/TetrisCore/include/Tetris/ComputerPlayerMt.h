#ifndef TETRIS_COMPUTERPLAYERMT_H_INCLUDED
#define TETRIS_COMPUTERPLAYERMT_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/GameStateNode.h"
#include <boost/shared_ptr.hpp>
#include <memory>


namespace Tetris
{

    class Evaluator;
    class WorkerPool;

    class ComputerPlayerMt
    {
    public:
        ComputerPlayerMt(boost::shared_ptr<WorkerPool> inWorkerPool,
                         std::auto_ptr<GameStateNode> inNode,
                         const BlockTypes & inBlockTypes,
                         const std::vector<int> & inWidths,
                         std::auto_ptr<Evaluator> inEvaluator);

        ~ComputerPlayerMt();
        
        void start();

        void stop();

        bool isFinished() const;

        int getCurrentSearchDepth() const;

        int getMaxSearchDepth() const;

        NodePtr result() const;

    private:
        ChildNodes getLayer1Nodes(BlockType inBlockType, size_t inWidth) const;

        int mMaxSearchDepth;
        NodePtr mRootNode;
        typedef boost::shared_ptr<ComputerPlayer> ComputerPlayerPtr;
        struct ComputerPlayerInfo
        {
            ComputerPlayerInfo(NodePtr inChildNode, ComputerPlayerPtr inComputerPlayerPtr) :
                mChildNode(inChildNode),
                mComputerPlayer(inComputerPlayerPtr)
            {
            }
            NodePtr mChildNode;
            ComputerPlayerPtr mComputerPlayer;
        };

        std::vector<ComputerPlayerInfo> mComputerPlayers;
        boost::shared_ptr<WorkerPool> mWorkerPool;
        boost::scoped_ptr<Evaluator> mEvaluator;
        mutable bool mFinished;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
