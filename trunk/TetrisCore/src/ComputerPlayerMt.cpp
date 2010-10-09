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

    ComputerPlayerMt::ComputerPlayerMt(WorkerPool * inWorkerPool,
                                   std::auto_ptr<GameStateNode> inNode,
                                   const BlockTypes & inBlockTypes,
                                   const std::vector<int> & inWidths,
                                   std::auto_ptr<Evaluator> inEvaluator) :
        mMaxSearchDepth(inWidths.size()),
        mRootNode(inNode.release()),
        mComputerPlayers(),
        mWorkerPool(inWorkerPool),
        mEvaluator(inEvaluator.release())
    {   
        ChildNodes layer1Nodes(getLayer1Nodes(inBlockTypes[0], inWidths[0]));
        ChildNodes::iterator it = layer1Nodes.begin(), end = layer1Nodes.end();
        for (; it != end; ++it)
        {
            NodePtr childNode(*it);
            mComputerPlayers.push_back(
                ComputerPlayerPtr(
                    new ComputerPlayer(
                        inWorkerPool->getWorker(),
                        childNode->clone(),
                        DropFirst(inBlockTypes),
                        DropFirst(inWidths),
                        mEvaluator->clone())));
        }
    }


    ComputerPlayerMt::~ComputerPlayerMt()
    {
        mWorkerPool->interruptAll();
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
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            if (!mComputerPlayers[idx]->isFinished())
            {
                return false;
            }
        }
        return true;
    }


    int ComputerPlayerMt::getCurrentSearchDepth() const
    {
        int result = 0;
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            result = std::min<int>(result, mComputerPlayers[idx]->getCurrentSearchDepth());
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
        LogInfo(MakeString() << "Search depth: " << searchDepth);
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            ComputerPlayer & comp = *mComputerPlayers[idx];
            ComputerPlayer::LayerData layerData;
            comp.getLayerData(searchDepth, layerData);
            if (!result || layerData.mBestChild->state().quality() > result->state().quality())
            {
                result = layerData.mBestChild;
            }
        }
        return result;
    }


    void ComputerPlayerMt::stop()
    {
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            mComputerPlayers[idx]->stop();
            LogInfo(MakeString() << "Computer " << idx << " stopped.");
        }
    }


    void ComputerPlayerMt::start()
    {
        for (size_t idx = 0; idx != mComputerPlayers.size(); ++idx)
        {
            mComputerPlayers[idx]->start();
            LogInfo(MakeString() << "Computer " << idx << " started.");
        }
    }

} // namespace Tetris
