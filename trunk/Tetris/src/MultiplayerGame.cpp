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
    virtual void onGameStateChanged(SimpleGame * )
    {
        // Not interested.
    }

    Player findPlayer(SimpleGame * inSimpleGame) const
    {
        Players::const_iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player player(**it);
            if (player.simpleGame() == inSimpleGame)
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
            Player player(**it);
            if (player.teamName() != activePlayer.teamName())
            {
                player.simpleGame()->applyLinePenalty(inLineCount);
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


Player * MultiplayerGame::join(std::auto_ptr<Player> inPlayer)
{
    mImpl->mPlayers.push_back(inPlayer.release());
    SimpleGame::RegisterEventHandler(mImpl->mPlayers.back()->simpleGame(), mImpl);
    return mImpl->mPlayers.back();
}


void MultiplayerGame::leave(Player * inPlayer)
{
    SimpleGame::UnregisterEventHandler(inPlayer->simpleGame(), mImpl);
    Players::iterator it = std::find(mImpl->mPlayers.begin(), mImpl->mPlayers.end(), inPlayer);
    if (it == mImpl->mPlayers.end())
    {
        throw std::runtime_error("This Player was not found in the list of players.");
    }
    mImpl->mPlayers.erase(it);
}


size_t MultiplayerGame::playerCount() const
{
    return mImpl->mPlayers.size();
}


const Player * MultiplayerGame::getPlayer(size_t inIndex) const
{
    return mImpl->mPlayers[inIndex];
}


Player * MultiplayerGame::getPlayer(size_t inIndex)
{
    return mImpl->mPlayers[inIndex];
}


} // namespace Tetris
