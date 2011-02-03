#include "Tetris/MultiplayerGame.h"
#include "Tetris/Game.h"
#include "Tetris/Logging.h"
#include "Tetris/Threading.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <set>


namespace Tetris {


struct MultiplayerGame::Impl : boost::noncopyable,
                               public Game::EventHandler
{
    typedef MultiplayerGame::Games Games;

    virtual void onGameStateChanged(Game * )
    {
        // Not interested.
    }

    virtual void onLinesCleared(Game * inGame, int inLineCount)
    {
        // If number of lines >= 2 then apply a line penalty to each non-allied player.
        // Note: we currently assume that all other players are non-allied.
        int numGames = mGames.size();
        int gameIndex = 0;
        Games::iterator it = mGames.begin(), end = mGames.end();
        for (; it != end; ++it)
        {
            ThreadSafe<Game> threadSafeGame(*it);
            ScopedReaderAndWriter<Game> rwgame(threadSafeGame);
            if (rwgame.get() != inGame)
            {
                LogInfo(MakeString() << "Penalty given to player. (Lines made: " << inLineCount << ")");
                rwgame.get()->applyLinePenalty(inLineCount);
            }
            gameIndex++;
        }
        assert(gameIndex == numGames);
    }

    Games mGames;
};


MultiplayerGame::MultiplayerGame() :
    mImpl(new Impl)
{
}


MultiplayerGame::~MultiplayerGame()
{
    delete mImpl;
}


void MultiplayerGame::join(ThreadSafe<Game> inGame)
{
    mImpl->mGames.insert(inGame);

    ScopedReaderAndWriter<Game> rwgame(inGame);
    rwgame->registerEventHandler(mImpl);
}


void MultiplayerGame::leave(ThreadSafe<Game> inGame)
{
    // calling erase(..) on a vector is slow,
    // but that should not be an issue here
    mImpl->mGames.erase(inGame);

    ScopedReaderAndWriter<Game> rwgame(inGame);
    rwgame->unregisterEventHandler(mImpl);
}


const MultiplayerGame::Games & MultiplayerGame::games() const
{
    return mImpl->mGames;
}


} // namespace Tetris
