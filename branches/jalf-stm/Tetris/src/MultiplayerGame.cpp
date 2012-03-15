#include "Tetris/MultiplayerGame.h"
#include "Tetris/Game.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <functional>
#include <vector>


namespace Tetris {


using namespace Futile;


struct MultiplayerGame::Impl : boost::noncopyable
{
    Impl(unsigned inRowCount, unsigned inColumnCount) :
        mRowCount(inRowCount),
        mColumnCount(inColumnCount)
    {
    }

    ~Impl()
    {
    }

    Player & addPlayer(PlayerType inPlayerType, const TeamName & inTeamName, const PlayerName & inPlayerName)
    {
        PlayerPtr playerPtr(Player::Create(inPlayerType, inTeamName, inPlayerName, mRowCount, mColumnCount));
        mPlayers.push_back(playerPtr);
        playerPtr->game().LinesCleared.connect(boost::bind(&Impl::onLinesCleared, this, _1, _2));
        return *playerPtr;
    }

    const Player & findPlayer(const SimpleGame & inGame) const
    {
        Players::const_iterator it = std::find_if(mPlayers.begin(),
                                                  mPlayers.end(),
                                                  [&](const PlayerPtr & playerPtr) { return &playerPtr->game() == &inGame; });
        if (it == mPlayers.end())
        {
            throw std::runtime_error("Player not found!");
        }
        PlayerPtr playerPtr(*it);
        return **it;
    }

    void onLinesCleared(const SimpleGame & inGame, std::size_t inLineCount)
    {
        // Penalty to oppononent start from 2 lines.
        if (inLineCount < 2)
        {
            return;
        }

        // If number of lines >= 2 then apply a line penalty to each non-allied player.
        const Player & activePlayer(findPlayer(inGame));

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


Player & MultiplayerGame::addHumanPlayer(const TeamName & inTeamName,
                                         const PlayerName & inPlayerName)
{
    return mImpl->addPlayer(PlayerType_Human, inTeamName, inPlayerName);
}


Player & MultiplayerGame::addComputerPlayer(const TeamName & inTeamName,
                                            const PlayerName & inPlayerName)
{
    return mImpl->addPlayer(PlayerType_Computer, inTeamName, inPlayerName);
}


void MultiplayerGame::removePlayer(Player * inPlayer)
{
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
