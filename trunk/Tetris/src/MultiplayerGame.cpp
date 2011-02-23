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
    Impl(size_t inRowCount, size_t inColumnCount) :
        mRowCount(inRowCount),
        mColumnCount(inColumnCount)
    {
    }

    ~Impl()
    {
    }


    virtual void onGameStateChanged(SimpleGame * )
    {
        // Not interested.
    }

    virtual void onDestroy(SimpleGame * )
    {
    }

    Player & findPlayer(SimpleGame * inSimpleGame) const
    {
        Players::const_iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player & player(**it);
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
        Player & activePlayer(findPlayer(inSimpleGame));

        Players::iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player & player(**it);
            if (player.teamName() != activePlayer.teamName())
            {
                player.simpleGame()->applyLinePenalty(inLineCount);
            }
        }
    }

    Players mPlayers;
    size_t mRowCount;
    size_t mColumnCount;
};


MultiplayerGame::MultiplayerGame(size_t inRowCount, size_t inColumnCount) :
    mImpl(new Impl(inRowCount, inColumnCount))
{
}


MultiplayerGame::~MultiplayerGame()
{
    delete mImpl;
}


Player * MultiplayerGame::addPlayer(PlayerType inPlayerType,
                                    const TeamName & inTeamName,
                                    const PlayerName & inPlayerName)
{
    mImpl->mPlayers.push_back(Player::Create(inPlayerType,
                                             inTeamName,
                                             inPlayerName,
                                             mImpl->mRowCount,
                                             mImpl->mColumnCount).release());
    Player * player = mImpl->mPlayers.back();
    SimpleGame::RegisterEventHandler(player->simpleGame(), mImpl);
    return player;
}


void MultiplayerGame::removePlayer(Player * inPlayer)
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
