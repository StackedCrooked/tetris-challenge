#include "Tetris/Config.h"
#include "Tetris/BlockMover.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Threading.h"
#include "Tetris/Assert.h"
#include "Poco/Stopwatch.h"
#include "Poco/Timer.h"


namespace Tetris
{

    class BlockMoverImpl
    {
    public:
        BlockMoverImpl(const ThreadSafe<Game> & inGame);

        ~BlockMoverImpl();

        void setSpeed(int inNumMovesPerSecond);

        int speed() const;

        void setInterval(int inTimeBetweenMovesInMilliseconds);

        int interval() const;

        void setCallback(const boost::function<void()> & inCallback);

    private:
        BlockMoverImpl(const BlockMoverImpl &);
        BlockMoverImpl & operator=(const BlockMoverImpl&);

        void onTimer(Poco::Timer & ioTimer);
        void move();

        ThreadSafe<Game> mGame;
        boost::function<void()> mCallback;
        boost::scoped_ptr<Poco::Timer> mTimer;
        Poco::Stopwatch mStopwatch;
        double mIntervalMs;
    };



    BlockMoverImpl::BlockMoverImpl(const ThreadSafe<Game> & inGame) :
        mGame(inGame),
        mCallback(),
        mTimer(),
        mStopwatch(),
        mIntervalMs(50)
    {
        int interval = std::min<int>(10, int(mIntervalMs/3));
        mTimer.reset(new Poco::Timer(0, interval));
        Poco::TimerCallback<BlockMoverImpl> callback(*this, &BlockMoverImpl::onTimer);
        mTimer->start(callback);
        mStopwatch.start();
    }


    BlockMoverImpl::~BlockMoverImpl()
    {
        mTimer->stop();
        mTimer.reset();
    }


    int BlockMoverImpl::speed() const
    {
        return static_cast<int>(0.5 + (1000.0 / mIntervalMs));
    }


    void BlockMoverImpl::setSpeed(int inNumMovesPerSecond)
    {
        mIntervalMs = 1000.0 / static_cast<double>(inNumMovesPerSecond);
        if (mTimer)
        {
            int interval = std::min<int>(10, int(mIntervalMs/3));
            mTimer->setPeriodicInterval(interval);
        }
    }


    void BlockMoverImpl::setInterval(int inTimeBetweenMovesInMilliseconds)
    {
        mIntervalMs = static_cast<double>(inTimeBetweenMovesInMilliseconds);
    }


    int BlockMoverImpl::interval() const
    {
        return static_cast<int>(0.5 + mIntervalMs);
    }


    void BlockMoverImpl::setCallback(const boost::function<void()> & inCallback)
    {
        mCallback = inCallback;
    }


    void BlockMoverImpl::onTimer(Poco::Timer &)
    {
        try
        {
            if (mStopwatch.elapsed() > 1000 * mIntervalMs)
            {
                mStopwatch.restart();
                move();
                if (mCallback)
                {
                    mCallback();
                }
            }
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Unanticipated exception thrown in BlockMoverImpl::move(). Details: " << inException.what());
        }
    }


    void BlockMoverImpl::move()
    {
        ScopedReaderAndWriter<Game> wGame(mGame);
        ComputerGame & game = dynamic_cast<ComputerGame&>(*wGame.get());

        const ChildNodes & children = game.currentNode()->children();
        if (children.empty())
        {
            return;
        }

        GameStateNode & firstChild = **children.begin();

        const Block & block = game.activeBlock();
        const Block & targetBlock = firstChild.gameState().originalBlock();
        Assert(block.type() == targetBlock.type());
        if (block.rotation() != targetBlock.rotation())
        {
            if (!game.rotate())
            {
                // Damn we can't rotate.
                // Give up on this block.
                game.drop();
            }
        }
        else if (block.column() < targetBlock.column())
        {
            if (!game.move(MoveDirection_Right))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game.drop();
            }
        }
        else if (block.column() > targetBlock.column())
        {
            if (!game.move(MoveDirection_Left))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game.drop();
            }
        }
        else
        {
            game.move(MoveDirection_Down);
        }
    }


    BlockMover::BlockMover(const ThreadSafe<Game> & inGame) :
        mImpl(new BlockMoverImpl(inGame))
    {
    }


    BlockMover::~BlockMover()
    {
        delete mImpl;
        mImpl = 0;
    }


    int BlockMover::speed() const
    {
        return mImpl->speed();
    }


    void BlockMover::setSpeed(int inNumMovesPerSecond)
    {
        mImpl->setSpeed(inNumMovesPerSecond);
    }


    int BlockMover::interval() const
    {
        return mImpl->interval();
    }


    void BlockMover::setCallback(const BlockMoverCallback & inBlockMoverCallback)
    {
        mImpl->setCallback(boost::bind(inBlockMoverCallback, this));
    }

} // namespace Tetris
