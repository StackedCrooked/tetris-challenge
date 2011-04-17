#include "Tetris/Config.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Game.h"
#include "Futile/Logging.h"
#include "Futile/Threading.h"
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

    Player * addPlayer(PlayerType inPlayerType, const TeamName & inTeamName, const PlayerName & inPlayerName);

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
    SimpleGame::RegisterEventHandler(player->simpleGame(), this);
    return player;
}


Player * MultiplayerGame::addHumanPlayer(const TeamName & inTeamName,
                                         const PlayerName & inPlayerName)
{
    return mImpl->addPlayer(PlayerType_Human, inTeamName, inPlayerName);
}


Player * MultiplayerGame::addComputerPlayer(const TeamName & inTeamName,
                                            const PlayerName & inPlayerName,
                                            ComputerPlayer::Tweaker * inTweaker)
{
    Player * result = mImpl->addPlayer(PlayerType_Computer, inTeamName, inPlayerName);
    ComputerPlayer & computerPlayer = dynamic_cast<ComputerPlayer&>(*result);
    computerPlayer.setTweaker(inTweaker);
    return result;
}


void MultiplayerGame::removePlayer(Player * inPlayer)
{
    SimpleGame::UnregisterEventHandler(inPlayer->simpleGame(), mImpl.get());
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
