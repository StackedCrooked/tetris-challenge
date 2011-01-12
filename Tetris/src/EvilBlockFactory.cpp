#include "Tetris/Config.h"
#include "Tetris/EvilBlockFactory.h"
#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Poco/Thread.h"


namespace Tetris {


EvilBlockFactory::EvilBlockFactory(ThreadSafe<Game> inGame) :
    mGame(inGame),
    mWorkerPool("EvilBlockFactory", 4)
{
}


BlockType EvilBlockFactory::getNext() const
{
    std::vector<BlockTypes> allBlockTypes;
    for (unsigned i = BlockType_Begin; i < BlockType_End; ++i)
    {
        for (unsigned j = BlockType_Begin; j < BlockType_End; ++j)
        {
            BlockTypes blockTypes;
            blockTypes.push_back(i);
            blockTypes.push_back(j);
            allBlockTypes.push_back(blockTypes);
        }
    }

    std::vector<boost::shared_ptr<NodeCalculator> > nodeCalculators;
    for (unsigned int i = 0; i < allBlockTypes.size(); ++i)
    {
        ScopedReader<Game> gameReader(mGame);
        const HumanGame & game(*gameReader.get());
        const BlockTypes & blockTypes(allBlockTypes[i]);
        std::vector<int> widths;
        widths.push_back(40);
        widths.push_back(40);
        std::auto_ptr<GameStateNode> node(game.currentNode()->endNode()->clone());
        std::auto_ptr<Evaluator> evaluator(CreatePoly<Evaluator, Balanced>());
        std::auto_ptr<NodeCalculator> nodeCalculator(new NodeCalculator(node, blockTypes, widths, evaluator, mWorkerPool));

        boost::shared_ptr<NodeCalculator> nodeCalculatorPtr(nodeCalculator.release());
        nodeCalculatorPtr->start();
        nodeCalculators.push_back(nodeCalculatorPtr);
    }


    std::map<int, BlockTypes> results;
    while (true)
    {
        unsigned int i = 0;
        for (; i < nodeCalculators.size(); ++i)
        {
            NodeCalculator & nodeCalculator(*nodeCalculators[i]);
            if (nodeCalculator.status() != NodeCalculator::Status_Finished)
            {
                break;
            }
        }
        if (i == nodeCalculators.size())
        {
            Poco::Thread::sleep(100);
            break;
        }
    }

    for (unsigned int i = 0; i < nodeCalculators.size(); ++i)
    {
        NodeCalculator & nodeCalculator(*nodeCalculators[i]);
        NodePtr result = nodeCalculator.result();
        results.insert(std::make_pair(result->quality(), allBlockTypes[i]));
    }

    BlockTypes & blockTypes = results.begin()->second;
    return blockTypes[0];
}


} // namespace Tetris
