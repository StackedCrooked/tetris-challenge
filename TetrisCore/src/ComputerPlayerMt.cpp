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

    ComputerPlayerMt::ComputerPlayerMt(boost::shared_ptr<WorkerPool> inWorkerPool, 
                                       std::auto_ptr<GameStateNode> inNode,
                                       const BlockTypes & inBlockTypes,
                                       const std::vector<int> & inWidths,
                                       std::auto_ptr<Evaluator> inEvaluator) :
        mMaxSearchDepth(inWidths.size()),
        mRootNode(inNode.release()),
        mComputerPlayers(),
        mWorkerPool(inWorkerPool),
        mEvaluator(inEvaluator.release()),
        mFinished(false)
    {   
        ChildNodes layer1Nodes(getLayer1Nodes(inBlockTypes[0], inWidths[0]));
        ChildNodes::iterator it = layer1Nodes.begin(), end = layer1Nodes.end();
        for (; it != end; ++it)
        {
            NodePtr childNode(*it);
            mComputerPlayers.push_back(ComputerPlayerInfo(childNode,
                                                          ComputerPlayerPtr(new ComputerPlayer(inWorkerPool->getWorker(),
                                                                                               childNode->clone(),
                                                                                               DropFirst(inBlockTypes),
                                                                                               DropFirst(inWidths),
                                                                                               mEvaluator->clone()))));
        }
    }


    ComputerPlayerMt::~ComputerPlayerMt()
    {
        mWorkerPool->interruptAndClearQueue();
    }


    ChildNodes ComputerPlayerMt::getLayer1Nodes(BlockType inBlockType, size_t inWidth) const
    {   
        ChildNodes result(GameStateComparisonFunctor(mEvaluator->clone()));
        ChildNodes generatedChildNodes(GameStateComparisonFunctor(mEvaluator->clone()));
        GenerateOffspring(mRootNode, inBlockType, *mEvaluator, generatedChildNodes);
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


    bool ComputerPlayerMt::isFinished() const
    {
        if (!mFinished)
        {
            size_t idx = 0;
            size_t numComputers = mComputerPlayers.size();
            for (; idx != numComputers; ++idx)
            {
                if (!mComputerPlayers[idx].mComputerPlayer->isFinished())
                {
                    break;
                }
            }
            mFinished = (idx == numComputers);
        }
        return mFinished;
    }


    int ComputerPlayerMt::getCurrentSearchDepth() const
    {
        int result = 0;
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            int currentDepth = 1 + mComputerPlayers[idx].mComputerPlayer->getCurrentSearchDepth();
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


    int ComputerPlayerMt::getMaxSearchDepth() const
    {
        return mMaxSearchDepth;
    }


    NodePtr ComputerPlayerMt::result() const
    {
        NodePtr result;
        int searchDepth = getCurrentSearchDepth();
        int selected = 0;
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            const ComputerPlayerInfo & compInfo = mComputerPlayers[idx];
            NodePtr compResult = compInfo.mComputerPlayer->result();
            if (!result || compResult->endNode()->state().quality() > result->state().quality())
            {
                Assert(compInfo.mChildNode->children().empty());
                compInfo.mChildNode->children().insert(compResult);
                result = compInfo.mChildNode;
                selected = idx;
            }
        }
        return result;
    }


    void ComputerPlayerMt::stop()
    {
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            mComputerPlayers[idx].mComputerPlayer->stop();
        }
    }


    void ComputerPlayerMt::start()
    {
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            mComputerPlayers[idx].mComputerPlayer->start();
        }
    }

} // namespace Tetris
