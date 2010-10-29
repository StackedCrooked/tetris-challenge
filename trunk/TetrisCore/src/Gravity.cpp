#include "Tetris/Config.h"
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

        // Number of rows per second
        float currentSpeed() const;

        static float CalculateSpeed(int inLevel);

    private:
        GravityImpl(const GravityImpl &);
        GravityImpl & operator=(const GravityImpl &);

        void onTimerEvent(Poco::Timer & inTimer);

        Protected<Game> mThreadSafeGame;
        int mLevel;
        Poco::Timer mTimer;
    };


    GravityImpl::GravityImpl(const Protected<Game> & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mLevel(0)
    {
        ScopedConstAtom<Game> rgame(mThreadSafeGame);
        mTimer.start(Poco::TimerCallback<GravityImpl>(*this, &GravityImpl::onTimerEvent));
        mTimer.setPeriodicInterval(sIntervals[rgame->level()]);
    }


    GravityImpl::~GravityImpl()
    {
        mTimer.stop();
    }


    void GravityImpl::onTimerEvent(Poco::Timer & inTimer)
    {
        try
        {
            {
                ScopedAtom<Game> game(mThreadSafeGame, 1000);
                if (game->isGameOver())
                {
                    return;
                }
                game->move(Direction_Down);
                mLevel = game->level();
            }           
            mTimer.setPeriodicInterval(sIntervals[mLevel]);
        }
        catch (const std::exception & inException)
        {
            LogError(inException.what());
        }
    }


    float GravityImpl::currentSpeed() const
    {
        return CalculateSpeed(mLevel);
    }
    
    
    float GravityImpl::CalculateSpeed(int inLevel)
    {
        return static_cast<float>(1000.0 / static_cast<float>(sIntervals[inLevel]));
    }

    
    Gravity::Gravity(const Protected<Game> & inGame) :
        mImpl(new GravityImpl(inGame))
    {
    }

    
    float Gravity::currentSpeed() const
    {
        return mImpl->currentSpeed();
    }
    
    
    float Gravity::CalculateSpeed(int inLevel)
    {
        return GravityImpl::CalculateSpeed(inLevel);
    }

} // namespace Tetris
