#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


#include "Tetris/Game.h"
#include "Tetris/NodePtr.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Threading.h"


namespace Tetris
{

    class BlockMover;
    class Evaluator;
    class Gravity;
    class AbstractNodeCalculator;
    class Worker;
    class WorkerPool;
    typedef std::vector<int> Widths;


    class ComputerPlayer
    {
    public:
        ComputerPlayer(const Protected<Game> & inProtectedGame);

        void runImpl();


    private:
        int calculateRemainingTimeMs(Game & game) const;

        boost::shared_ptr<WorkerPool> mWorkerPool;
        Protected<Game> mProtectedGame;
        boost::scoped_ptr<AbstractNodeCalculator> mNodeCalculator;
        boost::scoped_ptr<Gravity> mGravity;
        boost::scoped_ptr<BlockMover> mBlockMover;
        boost::scoped_ptr<Evaluator> mEvaluator;
        int mCurrentSearchDepth;
        int mMaxSearchDepth;
        int mSearchWidth;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
