#ifndef TETRIS_COMPUTERPLAYERMT_H_INCLUDED
#define TETRIS_COMPUTERPLAYERMT_H_INCLUDED


#include "Tetris/ComputerPlayer.h"
#include "Tetris/GameStateNode.h"


namespace Tetris
{

    class Evaluator;
    class WorkerPool;

    class MoveCalculatorMt : public CompositeMoveCalculator
    {
    public:
        MoveCalculatorMt(boost::shared_ptr<WorkerPool> inWorkerPool,
                         std::auto_ptr<GameStateNode> inNode,
                         const BlockTypes & inBlockTypes,
                         const std::vector<int> & inWidths,
                         std::auto_ptr<Evaluator> inEvaluator);

        virtual ~MoveCalculatorMt();
        
        virtual void start();

        virtual void stop();

        virtual int getCurrentSearchDepth() const;

        virtual int getMaxSearchDepth() const;

        virtual MoveCalculator::Status status() const;

        virtual NodePtr result() const;

    private:
        ChildNodes getLayer1Nodes(BlockType inBlockType, size_t inWidth) const;

        NodePtr mRootNode;
        int mMaxSearchDepth;
        
        typedef boost::shared_ptr<ConcreteMoveCalculator> MoveCalculatorPtr;
        struct ComputerPlayerInfo
        {
            ComputerPlayerInfo(NodePtr inChildNode, MoveCalculatorPtr inComputerPlayerPtr) :
                mChildNode(inChildNode),
                mMoveCalculator(inComputerPlayerPtr)
            {
            }
            NodePtr mChildNode;
            MoveCalculatorPtr mMoveCalculator;
        };

        std::vector<ComputerPlayerInfo> mComputerPlayers;
        boost::scoped_ptr<Evaluator> mEvaluator;
        boost::shared_ptr<WorkerPool> mWorkerPool;
        mutable NodePtr mCachedResult;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
