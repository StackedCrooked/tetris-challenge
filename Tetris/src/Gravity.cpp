#include "Tetris/Config.h"
#include "Tetris/Gravity.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Game.h"
#include "Tetris/Direction.h"
#include "Tetris/Logging.h"
#include "Tetris/Threading.h"
#include "Tetris/MakeString.h"
#include "Tetris/Assert.h"
#include "Poco/Timer.h"
#include "Poco/Stopwatch.h"
#include <algorithm>


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

    const int cIntervalCount = sizeof(sIntervals)/sizeof(int);
    extern const int cMaxLevel = sizeof(sIntervals)/sizeof(int) - 1;


    class GravityImpl
    {
    public:
        GravityImpl(const ThreadSafe<Game> & inGame);

        ~GravityImpl();

        void setCallback(const boost::function<void()> & inCallback);

        // Number of rows per second
        double speed() const;

        static double CalculateSpeed(int inLevel);

        // Interval between two moves expressed in milliseconds.
        int interval() const;

    private:
        GravityImpl(const GravityImpl &);
        GravityImpl & operator=(const GravityImpl &);

        void onTimerEvent(Poco::Timer & inTimer);

        ThreadSafe<Game> mThreadSafeGame;
        boost::function<void()> mCallback;
        int mLevel;
        Poco::Timer mTimer;
        Poco::Stopwatch mStopwatch;
    };


    GravityImpl::GravityImpl(const ThreadSafe<Game> & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),        
        mCallback(),
        mLevel(0),
        mTimer(),
        mStopwatch()
    {
        ScopedReader<Game> rgame(mThreadSafeGame);
        mTimer.start(Poco::TimerCallback<GravityImpl>(*this, &GravityImpl::onTimerEvent));
        mTimer.setPeriodicInterval(sIntervals[rgame->level()]);
        mStopwatch.start();
    }


    GravityImpl::~GravityImpl()
    {
        mTimer.stop();
    }


    void GravityImpl::setCallback(const boost::function<void()> & inCallback)
    {
        mCallback = inCallback;
    }


    int GravityImpl::interval() const
    {
        return static_cast<int>(0.5 + speed() / 1000.0);
    }


    void GravityImpl::onTimerEvent(Poco::Timer & )
    {
        try
        {
            int oldLevel = mLevel;
            {
                if (mStopwatch.elapsed() > 1000  * interval())
                {
                    mStopwatch.restart();
                    ScopedReaderAndWriter<Game> game(mThreadSafeGame);
                    if (game->isGameOver())
                    {
                        return;
                    }
                    game->move(Direction_Down);
                    int maxLevel = cMaxLevel;
                    mLevel = std::min<int>(maxLevel, game->level());
                }
            }
            if (mLevel != oldLevel)
            {
                Assert(mLevel < cIntervalCount);
                int newLevel = sIntervals[mLevel];
                LogInfo(MakeString() << "Set level to " << newLevel << ".");
                mTimer.setPeriodicInterval(newLevel);
            }

            if (mCallback)
            {
                mCallback();
            }
        }
        catch (const std::exception & inException)
        {
            LogError(inException.what());
        }
    }


    double GravityImpl::speed() const
    {
        return CalculateSpeed(mLevel);
    }
    
    
    double GravityImpl::CalculateSpeed(int inLevel)
    {
        return static_cast<double>(1000.0 / static_cast<double>(sIntervals[inLevel]));
    }

    
    Gravity::Gravity(const ThreadSafe<Game> & inGame) :
        mImpl(new GravityImpl(inGame))
    {
    }


    Gravity::~Gravity()
    {
        delete mImpl;
        mImpl = 0;
    }


    void Gravity::setCallback(const GravityCallback & inGravityCallback)
    {
        mImpl->setCallback(boost::bind(inGravityCallback, this));
    }

    
    double Gravity::speed() const
    {
        return mImpl->speed();
    }
    
    
    double Gravity::CalculateSpeed(int inLevel)
    {
        return GravityImpl::CalculateSpeed(inLevel);
    }

} // namespace Tetris
