#include "Tetris/Gravity.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Game.h"
#include "Tetris/Direction.h"
#include "Tetris/Logging.h"
#include "Tetris/Threading.h"
#include "Poco/Timer.h"


namespace Tetris
{

    // Number of milliseconds between two drops.
    static const int sIntervals[] =
    {
        887, 820, 753, 686, 619,
        552, 469, 368, 285, 184,
        167, 151, 134, 117, 100,
        100, 84, 84, 67, 67, 50
    };


    static const int cMaxLevel = sizeof(sIntervals)/sizeof(int) - 1;


    class GravityImpl
    {
    public:
        GravityImpl(const Protected<Game> & inGame);

        ~GravityImpl();

        int getLevel() const;

        // Number of rows per second
        float currentSpeed() const;

        // The higher the level the faster the blocks are falling.
        // Levels go from 0 to 20. If the level is set to a
        // bigger value then it is set to 20 instead.
        void setLevel(int inLevel);

    private:
        GravityImpl(const GravityImpl &);
        GravityImpl & operator=(const GravityImpl &);

        void onTimerEvent(Poco::Timer & inTimer);

        Protected<Game> mThreadSafeGame;
        int mLevel;
        mutable boost::mutex mLevelMutex;
        Poco::Timer mTimer;
    };


    GravityImpl::GravityImpl(const Protected<Game> & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mLevel(0)
    {
        mTimer.start(Poco::TimerCallback<GravityImpl>(*this, &GravityImpl::onTimerEvent));
        mTimer.setPeriodicInterval(sIntervals[getLevel()]);
    }


    GravityImpl::~GravityImpl()
    {
        mTimer.stop();
    }


    void GravityImpl::onTimerEvent(Poco::Timer & inTimer)
    {
        try
        {
            int level = 0;
            // Scoped lock
            {
                ScopedAtom<Game> game(mThreadSafeGame, 1000);
                if (!game->isGameOver())
                {
                    game->move(Direction_Down);
                }
                level = game->currentNode()->state().stats().numLines() / 10;
            }

            if (level > getLevel() && level <= cMaxLevel)
            {
                setLevel(level);
                mTimer.setPeriodicInterval(sIntervals[level]);
            }
        }
        catch (const std::exception & inException)
        {
            LogError(inException.what());
        }
    }


    float GravityImpl::currentSpeed() const
    {
        return static_cast<float>(1000.0 / static_cast<float>(sIntervals[getLevel()]));
    }


    int GravityImpl::getLevel() const
    {
        boost::mutex::scoped_lock lock(mLevelMutex);
        return mLevel;
    }


    void GravityImpl::setLevel(int inLevel)
    {
        boost::mutex::scoped_lock lock(mLevelMutex);
        mLevel = std::min<int>(inLevel, cMaxLevel);
        mTimer.setPeriodicInterval(sIntervals[mLevel]);
    }

    
    Gravity::Gravity(const Protected<Game> & inGame) :
        mImpl(new GravityImpl(inGame))
    {
    }

    
    Gravity::~Gravity()
    {
        delete mImpl;
        mImpl = 0;
    }

    
    int Gravity::getLevel() const
    {
        return mImpl->getLevel();
    }


    void Gravity::setLevel(int inLevel)
    {
        mImpl->setLevel(inLevel);
    }


    float Gravity::currentSpeed() const
    {
        return mImpl->currentSpeed();
    }

} // namespace Tetris
