#if 0 // File disabled


#ifndef TETRIS_EVILBLOCKFACTORY_H
#define TETRIS_EVILBLOCKFACTORY_H


#include "Tetris/BlockFactory.h"
#include "Tetris/Game.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/Threading.h"
#include "Tetris/WorkerPool.h"
#include <boost/scoped_ptr.hpp>


namespace Tetris {


class EvilBlockFactory : public AbstractBlockFactory
{
public:
    EvilBlockFactory(ThreadSafe<Game> inGame);

    virtual BlockType getNext() const;

private:
    std::auto_ptr<NodeCalculator> createNodeCalculator(const BlockTypes & inBlockTypes);

    ThreadSafe<Game> mGame;
    WorkerPool mWorkerPool;
};


} // namespace Tetris


#endif // TETRIS_EVILBLOCKFACTORY_H



#endif // 0
