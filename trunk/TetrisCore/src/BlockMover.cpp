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
        BlockMoverImpl(const Protected<Game> & inGame, int inNumMovesPerSecond);

        ~BlockMoverImpl();

        void setSpeed(int inNumMovesPerSecond);

        int speed() const;

        void setInterval(int inTimeBetweenMovesInMilliseconds);

        int interval() const;

    private:
        BlockMoverImpl(const BlockMoverImpl &);
        BlockMoverImpl & operator=(const BlockMoverImpl&);

        void onTimer(Poco::Timer & ioTimer);
        void move();

        Protected<Game> mGame;
        boost::scoped_ptr<Poco::Timer> mTimer;
        Poco::Stopwatch mStopwatch;
        double mInterval;
    };



    BlockMoverImpl::BlockMoverImpl(const Protected<Game> & inGame, int inInterval) :
        mGame(inGame),
        mTimer(),
        mInterval(static_cast<double>(inInterval))
    {
        mTimer.reset(new Poco::Timer(0, 1));
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
        return static_cast<int>(0.5 + (1000.0 / mInterval));
    }


    void BlockMoverImpl::setSpeed(int inNumMovesPerSecond)
    {
        mInterval = 1000.0 / static_cast<double>(inNumMovesPerSecond);
    }
    

    void BlockMoverImpl::setInterval(int inTimeBetweenMovesInMilliseconds)
    {
        mInterval = static_cast<double>(inTimeBetweenMovesInMilliseconds);
    }


    int BlockMoverImpl::interval() const
    {
        return static_cast<int>(0.5 + mInterval);
    }

    void BlockMoverImpl::onTimer(Poco::Timer & ioTimer)
    {
        try
        {
            if (mStopwatch.elapsed() > 1000 * mInterval)
            {
                mStopwatch.restart();
                move();
            }
        }
        catch (const LockTimeout & inException)
        {
            LogWarning(MakeString() << "BlockMoverImpl failed to lock the Game due to timeout. Details: " << inException.what());
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Unanticipated exception thrown in BlockMoverImpl::move(). Details: " << inException.what());
        }
    }


    void BlockMoverImpl::move()
    {
        ScopedAtom<Game> wgame(mGame);
        Game & game = *wgame.get();

        const ChildNodes & children = game.currentNode()->children();
        if (children.empty())
        {
            return;
        }

        GameStateNode & firstChild = **children.begin();

        const Block & block = game.activeBlock();
        const Block & targetBlock = firstChild.state().originalBlock();
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
            if (!game.move(Direction_Right))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game.drop();
            }
        }
        else if (block.column() > targetBlock.column())
        {
            if (!game.move(Direction_Left))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game.drop();
            }
        }
        else
        {
            game.move(Direction_Down);
        }
    }


    BlockMover::BlockMover(const Protected<Game> & inGame, int inNumMovesPerSecond) :
        mImpl(new BlockMoverImpl(inGame, inNumMovesPerSecond))
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


} // namespace Tetris
