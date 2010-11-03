#include "Tetris/Config.h"
#include "Tetris/Gravity.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Game.h"
#include "Tetris/Direction.h"
#include "Tetris/Logging.h"
#include "Tetris/Threading.h"
#include "Poco/Timer.h"
#include "Poco/Stopwatch.h"


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

        // Number of rows per second
        double speed() const;

        static double CalculateSpeed(int inLevel);

        // Interval between two moves expressed in milliseconds.
        int interval() const;

    private:
        GravityImpl(const GravityImpl &);
        GravityImpl & operator=(const GravityImpl &);

        void onTimerEvent(Poco::Timer & inTimer);

        Protected<Game> mThreadSafeGame;
        int mLevel;
        Poco::Timer mTimer;
        Poco::Stopwatch mStopwatch;
    };


    GravityImpl::GravityImpl(const Protected<Game> & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mLevel(0)
    {
        ScopedConstAtom<Game> rgame(mThreadSafeGame);
        mTimer.start(Poco::TimerCallback<GravityImpl>(*this, &GravityImpl::onTimerEvent));
        mTimer.setPeriodicInterval(sIntervals[rgame->level()]);
        mStopwatch.start();
    }


    GravityImpl::~GravityImpl()
    {
        mTimer.stop();
    }


    int GravityImpl::interval() const
    {
        return static_cast<int>(0.5 + speed() / 1000.0);
    }


    void GravityImpl::onTimerEvent(Poco::Timer & inTimer)
    {
        try
        {
            {
                if (mStopwatch.elapsed() > 1000  * interval())
                {
                    mStopwatch.restart();
                    ScopedAtom<Game> game(mThreadSafeGame);
                    if (game->isGameOver())
                    {
                        return;
                    }
                    game->move(Direction_Down);
                    mLevel = game->level();
                }
            }           
            mTimer.setPeriodicInterval(sIntervals[mLevel]);
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

    
    Gravity::Gravity(const Protected<Game> & inGame) :
        mImpl(new GravityImpl(inGame))
    {
    }


    Gravity::~Gravity()
    {
        delete mImpl;
        mImpl = 0;
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
