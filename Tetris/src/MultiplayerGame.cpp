#include "Tetris/MultiplayerGame.h"
#include "Tetris/Game.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Threading.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>


namespace Tetris {


using namespace Futile;


struct MultiplayerGame::Impl : public SimpleGame::EventHandler,
                               boost::noncopyable
{
    Impl(unsigned inRowCount, unsigned inColumnCount) :
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

    Player & findPlayer(SimpleGame * inGame) const
    {
        Players::const_iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player & player(**it);
            if (&player.game() == inGame)
            {
                return player;
            }
        }
        throw std::runtime_error("Player not found!");
    }

    virtual void onLinesCleared(SimpleGame * inGame, std::size_t inLineCount)
    {
        // Penalty to oppononent start from 2 lines.
        if (inLineCount < 2)
        {
            return;
        }

        // If number of lines >= 2 then apply a line penalty to each non-allied player.
        Player & activePlayer(findPlayer(inGame));

        Players::iterator it = mPlayers.begin(), end = mPlayers.end();
        for (; it != end; ++it)
        {
            Player & player(**it);
            if (player.teamName() != activePlayer.teamName())
            {
                player.game().applyLinePenalty(inLineCount == 4 ? 4 : (inLineCount - 1));
            }
        }
    }

    typedef boost::shared_ptr<Player> PlayerPtr;
    typedef std::vector<PlayerPtr> Players;
    Players mPlayers;
    unsigned mRowCount;
    unsigned mColumnCount;
};


MultiplayerGame::MultiplayerGame(unsigned inRowCount, unsigned inColumnCount) :
    mImpl(new Impl(inRowCount, inColumnCount))
{
}


MultiplayerGame::~MultiplayerGame()
{
    // Don't allow exceptions to escape from the destructor.
    try
    {
        mImpl.reset();
    }
    catch (const std::exception & exc)
    {
        LogError(SS() << "~MultiplayerGame throws: " << exc.what());
    }
}


Player * MultiplayerGame::Impl::addPlayer(PlayerType inPlayerType,
                                          const TeamName & inTeamName,
                                          const PlayerName & inPlayerName)
{
    PlayerPtr playerPtr(Player::Create(inPlayerType,
                                       inTeamName,
                                       inPlayerName,
                                       mRowCount,
                                       mColumnCount));
    mPlayers.push_back(playerPtr);
    playerPtr->game().registerEventHandler(this);
    return playerPtr.get();
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
    inPlayer->game().unregisterEventHandler(mImpl.get());
    Impl::Players::iterator it = std::find_if(mImpl->mPlayers.begin(), mImpl->mPlayers.end(), boost::bind(&Impl::PlayerPtr::get, _1) == inPlayer);
    if (it == mImpl->mPlayers.end())
    {
        throw std::runtime_error("This Player was not found in the list of players.");
    }
    mImpl->mPlayers.erase(it);
}


unsigned MultiplayerGame::playerCount() const
{
    return mImpl->mPlayers.size();
}


const Player * MultiplayerGame::getPlayer(unsigned inIndex) const
{
    return mImpl->mPlayers[inIndex].get();
}


Player * MultiplayerGame::getPlayer(unsigned inIndex)
{
    return mImpl->mPlayers[inIndex].get();
}


unsigned MultiplayerGame::rowCount() const
{
    return mImpl->mRowCount;
}


unsigned MultiplayerGame::columnCount() const
{
    return mImpl->mRowCount;
}


} // namespace Tetris
