#include "Tetris/ComputerPlayerMt.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Assert.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/Logger.h"
#include "Tetris/MakeString.h"
#include "Tetris/WorkerPool.h"
#include <boost/bind.hpp>
#include <algorithm>


namespace Tetris
{

    template<class T>
    static T DropFirst(const T & inValue)
    {
        return T(inValue.begin() + 1, inValue.end());
    }

    MoveCalculatorMt::MoveCalculatorMt(boost::shared_ptr<WorkerPool> inWorkerPool, 
                                       std::auto_ptr<GameStateNode> inNode,
                                       const BlockTypes & inBlockTypes,
                                       const std::vector<int> & inWidths,
                                       std::auto_ptr<Evaluator> inEvaluator) :
        CompositeMoveCalculator(),
        mRootNode(inNode.release()),
        mMaxSearchDepth(inWidths.size()),
        mComputerPlayers(),
        mEvaluator(inEvaluator.release()),
        mWorkerPool(inWorkerPool)
    {   
        ChildNodes layer1Nodes(getLayer1Nodes(inBlockTypes[0], inWidths[0]));
        ChildNodes::iterator it = layer1Nodes.begin(), end = layer1Nodes.end();
        for (; it != end; ++it)
        {
            NodePtr childNode(*it);
            Assert(childNode->children().empty());
            MoveCalculatorPtr calc(new ConcreteMoveCalculator(inWorkerPool->getWorker(),
                                                              childNode->clone(),
                                                              DropFirst(inBlockTypes),
                                                              DropFirst(inWidths),
                                                              mEvaluator->clone()));
            ComputerPlayerInfo compInfo(childNode, calc);
            mComputerPlayers.push_back(compInfo);
        }
    }


    MoveCalculatorMt::~MoveCalculatorMt()
    {
        mWorkerPool->interruptAndClearQueue();
    }


    ChildNodes MoveCalculatorMt::getLayer1Nodes(BlockType inBlockType, size_t inWidth) const
    {   
        ChildNodes result(GameStateComparisonFunctor(mEvaluator->clone()));
        ChildNodes generatedChildNodes(GameStateComparisonFunctor(mEvaluator->clone()));
        GenerateOffspring(mRootNode, inBlockType, *mEvaluator, generatedChildNodes);
        Assert(mRootNode->children().empty());
        size_t count = 0;
        ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
        while (it != end && count < inWidth)
        {
            result.insert(*it);
            ++count;
            ++it;
        }
        return result;
    }


    MoveCalculator::Status MoveCalculatorMt::status() const
    {
        Status result = static_cast<Status>(Status_End - 1);
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            Status compStatus = mComputerPlayers[idx].mMoveCalculator->status();
            if (compStatus < result)
            {
                result = compStatus;
            }
        }
        return result;
    }


    int MoveCalculatorMt::getCurrentSearchDepth() const
    {
        int result = 0;
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            int currentDepth = 1 + mComputerPlayers[idx].mMoveCalculator->getCurrentSearchDepth();
            if (idx == 0)
            {
                result = currentDepth;
            }
            else
            {
                result = std::min<int>(result, currentDepth);
            }
        }
        return result;
    }


    int MoveCalculatorMt::getMaxSearchDepth() const
    {
        return mMaxSearchDepth;
    }


    NodePtr MoveCalculatorMt::result() const
    {
        Assert(status() == Status_Finished);
        if (!mCachedResult)
        {
            Assert(mRootNode->children().empty());
            NodePtr result;
            int searchDepth = getCurrentSearchDepth();
            int selected = 0;
            for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
            {
                const ComputerPlayerInfo & compInfo = mComputerPlayers[idx];
                Assert(compInfo.mMoveCalculator->status() == ConcreteMoveCalculator::Status_Finished);
                NodePtr compResult = compInfo.mMoveCalculator->result();
                if (!result || compResult->endNode()->state().quality(*mEvaluator) > result->state().quality(*mEvaluator))
                {
                    Assert(compInfo.mChildNode->children().empty());
                    compInfo.mChildNode->children().insert(compResult);
                    result = compInfo.mChildNode;
                    selected = idx;
                }
            }
            mCachedResult = result;
        }
        return mCachedResult;
    }


    void MoveCalculatorMt::stop()
    {
        LogInfo(MakeString() << "MoveCalculatorMt::stop(). Search depth: " << getCurrentSearchDepth() << "/" << getMaxSearchDepth());
        mWorkerPool->interruptAndClearQueue();
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            mComputerPlayers[idx].mMoveCalculator->stop();
        }
    }


    void MoveCalculatorMt::start()
    {
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            mComputerPlayers[idx].mMoveCalculator->start();
        }
    }

} // namespace Tetris
