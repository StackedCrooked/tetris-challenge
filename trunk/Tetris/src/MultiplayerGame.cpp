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


struct MultiplayerGame::Impl : public SimpleGame::EventHandler,
                               boost::noncopyable
{
    typedef MultiplayerGame::Players Players;

    virtual void onGameStateChanged(SimpleGame * )
    {
        // Not interested.
    }

    Player findPlayer(SimpleGame * inSimpleGame) const
    {
        Players::const_iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player player(*it);
            if (&player.simpleGame() == inSimpleGame)
            {
                return player;
            }
        }
        throw std::runtime_error("Player not found!");
    }

    virtual void onLinesCleared(SimpleGame * inSimpleGame, int inLineCount)
    {
        // If number of lines >= 2 then apply a line penalty to each non-allied player.
        Player activePlayer(findPlayer(inSimpleGame));

        Players::iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player player(*it);
            if (player.teamName() != activePlayer.teamName())
            {
                player.simpleGame().applyLinePenalty(inLineCount);
            }
        }
    }

    Players mPlayers;
};


MultiplayerGame::MultiplayerGame() :
    mImpl(new Impl)
{
}


MultiplayerGame::~MultiplayerGame()
{
    delete mImpl;
}


void MultiplayerGame::join(Player inPlayer)
{
    mImpl->mPlayers.insert(inPlayer);
    inPlayer.simpleGame().registerEventHandler(mImpl);
}


void MultiplayerGame::leave(Player inPlayer)
{
    inPlayer.simpleGame().unregisterEventHandler(mImpl);
    mImpl->mPlayers.erase(inPlayer);
}


const MultiplayerGame::Players & MultiplayerGame::players() const
{
    return mImpl->mPlayers;
}


} // namespace Tetris
