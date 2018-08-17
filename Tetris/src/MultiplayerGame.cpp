#include "Tetris/Config.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/GameImpl.h"
#include "Futile/Logging.h"
#include "Futile/Threading.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <set>


namespace Tetris {


struct MultiplayerGame::Impl final :
    Game::EventHandler,
    boost::noncopyable
{
    Impl(std::size_t inRowCount, std::size_t inColumnCount) :
        mRowCount(inRowCount),
        mColumnCount(inColumnCount)
    {
    }

    virtual ~Impl() { }

    virtual void onGameStateChanged(Game * )
    {
        // Not interested.
    }

    Player * addPlayer(PlayerType inPlayerType, const TeamName & inTeamName, const PlayerName & inPlayerName);

    Player & findPlayer(Game * inGame) const
    {
        Players::const_iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player & player(**it);
            if (player.simpleGame() == inGame)
            {
                return player;
            }
        }
        throw std::runtime_error("Player not found!");
    }

    virtual void onLinesCleared(Game * inGame, int inLineCount)
    {
        // If number of lines >= 2 then apply a line penalty to each non-allied player.
        Player & activePlayer(findPlayer(inGame));

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
    std::size_t mRowCount;
    std::size_t mColumnCount;
};


MultiplayerGame::MultiplayerGame(std::size_t inRowCount, std::size_t inColumnCount) :
    mImpl(new Impl(inRowCount, inColumnCount))
{
}


MultiplayerGame::~MultiplayerGame()
{
    mImpl.reset();
}


Player * MultiplayerGame::Impl::addPlayer(PlayerType inPlayerType,
                                          const TeamName & inTeamName,
                                          const PlayerName & inPlayerName)
{
    mPlayers.push_back(Player::Create(inPlayerType,
                                      inTeamName,
                                      inPlayerName,
                                      mRowCount,
                                      mColumnCount).release());
    Player * player = mPlayers.back();
    Game::RegisterEventHandler(player->simpleGame(), this);
    return player;
}


Player * MultiplayerGame::addHumanPlayer(const TeamName & inTeamName,
                                         const PlayerName & inPlayerName)
{
    return mImpl->addPlayer(Human, inTeamName, inPlayerName);
}


Player * MultiplayerGame::addComputerPlayer(const TeamName & inTeamName,
                                            const PlayerName & inPlayerName,
                                            ComputerPlayer::Tweaker * inTweaker)
{
    Player * result = mImpl->addPlayer(Computer, inTeamName, inPlayerName);
    ComputerPlayer & computerPlayer = dynamic_cast<ComputerPlayer&>(*result);
    computerPlayer.setTweaker(inTweaker);
    return result;
}


void MultiplayerGame::removePlayer(Player * inPlayer)
{
    Game::UnregisterEventHandler(inPlayer->simpleGame(), mImpl.get());
    Players::iterator it = std::find(mImpl->mPlayers.begin(), mImpl->mPlayers.end(), inPlayer);
    if (it == mImpl->mPlayers.end())
    {
        throw std::runtime_error("This Player was not found in the list of players.");
    }
    mImpl->mPlayers.erase(it);
}


std::size_t MultiplayerGame::playerCount() const
{
    return mImpl->mPlayers.size();
}


const Player * MultiplayerGame::getPlayer(std::size_t inIndex) const
{
    return mImpl->mPlayers[inIndex];
}


Player * MultiplayerGame::getPlayer(std::size_t inIndex)
{
    return mImpl->mPlayers[inIndex];
}


} // namespace Tetris
